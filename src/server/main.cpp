#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>

// 处理服务器ctrl+c结束后，重置user的状态
void resetHandler(int) {
    ChatService::instance()->reset();
    std::exit(0);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000"
                  << std::endl;
        std::exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 注册信号处理器
    signal(SIGINT, resetHandler);

    // 创建事件循环
    EventLoop loop;
    // 创建服务器地址
    InetAddress addr(ip, port);
    // 创建聊天服务器对象
    ChatServer server(&loop, addr, "ChatServer");

    // 启动服务
    server.start();
    // 开启事件循环
    loop.loop();

    return 0;
}