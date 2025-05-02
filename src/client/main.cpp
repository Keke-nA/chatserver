#include "json.hpp"
#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
using json = nlohmann::json;

#include <arpa/inet.h>
#include <atomic>
#include <netinet/in.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "group.hpp"
#include "public.hpp"
#include "user.hpp"

// 全局变量
User g_currentUser;                        // 当前登录用户信息
std::vector<User> g_currentUserFriendList; // 当前用户好友列表
std::vector<Group> g_currentUserGroupList; // 当前用户群组列表
bool isMainMenuRunning = false;            // 控制主菜单运行状态
sem_t rwsem;                               // 读写线程通信信号量
std::atomic_bool g_isLoginSuccess{false};  // 登录状态

// 函数声明
void readTaskHandler(int clientfd);
std::string getCurrentTime();
void mainMenu(int);
void showCurrentUserData();

// 聊天客户端主程序
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000"
                  << std::endl;
        exit(-1);
    }

    // 解析命令行参数
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd) {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }

    // 配置服务器地址
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // 连接服务器
    if (-1 == connect(clientfd, (sockaddr*)&server, sizeof(sockaddr_in))) {
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化信号量
    sem_init(&rwsem, 0, 0);

    // 启动接收线程
    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    // 主线程处理用户输入
    for (;;) {
        // 显示菜单
        std::cout << "========================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "choice:";
        int choice = 0;

        // 检查输入有效性
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "invalid input! Please enter a number." << std::endl;
            continue;
        }
        std::cin.get(); // 清除回车

        switch (choice) {
        case 1: { // 登录
            int id = 0;
            char pwd[50] = {0};
            std::cout << "userid:";
            std::cin >> id;
            std::cin.get();
            std::cout << "userpassword:";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            std::string request = js.dump();

            g_isLoginSuccess = false;
            int len =
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1) {
                std::cerr << "send login msg error:" << request << std::endl;
            }

            sem_wait(&rwsem); // 等待子线程处理登录响应

            if (g_isLoginSuccess) {
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
            break;
        }
        case 2: { // 注册
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout << "username:";
            std::cin.getline(name, 50);
            std::cout << "userpassword:";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            std::string request = js.dump();

            int len =
                send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1) {
                std::cerr << "send reg msg error:" << request << std::endl;
            }

            sem_wait(&rwsem); // 等待子线程处理注册响应
            break;
        }
        case 3: // 退出
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        default:
            std::cerr << "invalid input!" << std::endl;
            break;
        }
    }

    return 0;
}

// 处理注册响应
void doRegResponse(json& responsejs) {
    if (0 != responsejs["errno"].get<int>()) {
        std::cerr << "name is already exist, register error!" << std::endl;
    } else {
        std::cout << "name register success, userid is " << responsejs["id"]
                  << ", do not forget it!" << std::endl;
    }
}

// 处理登录响应
void doLoginResponse(json& responsejs) {
    if (0 != responsejs["errno"].get<int>()) {
        std::cerr << responsejs["errmsg"] << std::endl;
        g_isLoginSuccess = false;
    } else {
        // 记录用户信息
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录好友列表
        if (responsejs.contains("friends")) {
            g_currentUserFriendList.clear();
            std::vector<std::string> vec = responsejs["friends"];
            for (std::string& str : vec) {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录群组列表
        if (responsejs.contains("groups")) {
            g_currentUserGroupList.clear();
            std::vector<std::string> vec1 = responsejs["groups"];
            for (std::string& groupstr : vec1) {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                std::vector<std::string> vec2 = grpjs["users"];
                for (std::string& userstr : vec2) {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }

        // 显示用户信息
        showCurrentUserData();

        // 显示离线消息
        if (responsejs.contains("offlinemsg")) {
            std::vector<std::string> vec = responsejs["offlinemsg"];
            for (std::string& str : vec) {
                json js = json::parse(str);
                if (ONE_CHAT_MSG == js["msgid"].get<int>()) {
                    std::cout << js["time"].get<std::string>() << " ["
                              << js["id"] << "]"
                              << js["name"].get<std::string>()
                              << " said: " << js["msg"].get<std::string>()
                              << std::endl;
                } else {
                    std::cout << "群消息[" << js["groupid"]
                              << "]:" << js["time"].get<std::string>() << " ["
                              << js["id"] << "]"
                              << js["name"].get<std::string>()
                              << " said: " << js["msg"].get<std::string>()
                              << std::endl;
                }
            }
        }
        g_isLoginSuccess = true;
    }
}

// 接收线程处理函数
void readTaskHandler(int clientfd) {
    for (;;) {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (-1 == len || 0 == len) {
            close(clientfd);
            exit(-1);
        }

        // 解析接收的JSON数据
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype) {
            std::cout << js["time"].get<std::string>() << " [" << js["id"]
                      << "]" << js["name"].get<std::string>()
                      << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype) {
            std::cout << "群消息[" << js["groupid"]
                      << "]:" << js["time"].get<std::string>() << " ["
                      << js["id"] << "]" << js["name"].get<std::string>()
                      << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype) {
            doLoginResponse(js);
            sem_post(&rwsem);
            continue;
        }

        if (REG_MSG_ACK == msgtype) {
            doRegResponse(js);
            sem_post(&rwsem);
            continue;
        }
    }
}

// 显示当前用户信息
void showCurrentUserData() {
    std::cout << "======================login user======================"
              << std::endl;
    std::cout << "current login user => id:" << g_currentUser.getId()
              << " name:" << g_currentUser.getName() << std::endl;
    std::cout << "----------------------friend list---------------------"
              << std::endl;
    if (!g_currentUserFriendList.empty()) {
        for (User& user : g_currentUserFriendList) {
            std::cout << user.getId() << " " << user.getName() << " "
                      << user.getState() << std::endl;
        }
    }
    std::cout << "----------------------group list----------------------"
              << std::endl;
    if (!g_currentUserGroupList.empty()) {
        for (Group& group : g_currentUserGroupList) {
            std::cout << group.getId() << " " << group.getName() << " "
                      << group.getDesc() << std::endl;
            for (GroupUser& user : group.getUsers()) {
                std::cout << user.getId() << " " << user.getName() << " "
                          << user.getState() << " " << user.getRole()
                          << std::endl;
            }
        }
    }
    std::cout << "======================================================"
              << std::endl;
}

// 命令处理函数声明
void help(int fd = 0, std::string str = "");
void chat(int, std::string);
void addfriend(int, std::string);
void creategroup(int, std::string);
void addgroup(int, std::string);
void groupchat(int, std::string);
void loginout(int, std::string);

// 命令映射表
std::unordered_map<std::string, std::string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 命令处理函数映射表
std::unordered_map<std::string, std::function<void(int, std::string)>>
    commandHandlerMap = {
        {"help", help},
        {"chat", chat},
        {"addfriend", addfriend},
        {"creategroup", creategroup},
        {"addgroup", addgroup},
        {"groupchat", groupchat},
        {"loginout", loginout}};

// 主聊天菜单
void mainMenu(int clientfd) {
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning) {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command;
        int idx = commandbuf.find(":");
        if (-1 == idx) {
            command = commandbuf;
            auto it = commandHandlerMap.find(command);
            if (it == commandHandlerMap.end()) {
                std::cerr << "invalid input command!" << std::endl;
                continue;
            }
            it->second(clientfd, "");
        } else {
            command = commandbuf.substr(0, idx);
            auto it = commandHandlerMap.find(command);
            if (it == commandHandlerMap.end()) {
                std::cerr << "invalid input command!" << std::endl;
                continue;
            }
            it->second(
                clientfd,
                commandbuf.substr(idx + 1, commandbuf.size() - idx));
        }
    }
}

// 显示帮助信息
void help(int, std::string) {
    std::cout << "show command list >>> " << std::endl;
    for (auto& p : commandMap) {
        std::cout << p.first << " : " << p.second << std::endl;
    }
    std::cout << std::endl;
}

// 添加好友
void addfriend(int clientfd, std::string str) {
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send addfriend msg error -> " << buffer << std::endl;
    }
}

// 私聊
void chat(int clientfd, std::string str) {
    int idx = str.find(":");
    if (-1 == idx) {
        std::cerr << "chat command invalid!" << std::endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send chat msg error -> " << buffer << std::endl;
    }
}

// 创建群组
void creategroup(int clientfd, std::string str) {
    int idx = str.find(":");
    if (-1 == idx) {
        std::cerr << "creategroup command invalid!" << std::endl;
        return;
    }

    std::string groupname = str.substr(0, idx);
    std::string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send creategroup msg error -> " << buffer << std::endl;
    }
}

// 加入群组
void addgroup(int clientfd, std::string str) {
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send addgroup msg error -> " << buffer << std::endl;
    }
}

// 群聊
void groupchat(int clientfd, std::string str) {
    int idx = str.find(":");
    if (-1 == idx) {
        std::cerr << "groupchat command invalid!" << std::endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send groupchat msg error -> " << buffer << std::endl;
    }
}

// 注销
void loginout(int clientfd, std::string) {
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send loginout msg error -> " << buffer << std::endl;
    } else {
        isMainMenuRunning = false;
    }
}

// 获取当前系统时间
std::string getCurrentTime() {
    auto tt =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(
        date,
        "%d-%02d-%02d %02d:%02d:%02d",
        (int)ptm->tm_year + 1900,
        (int)ptm->tm_mon + 1,
        (int)ptm->tm_mday,
        (int)ptm->tm_hour,
        (int)ptm->tm_min,
        (int)ptm->tm_sec);
    return std::string(date);
}