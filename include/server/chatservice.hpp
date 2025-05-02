#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "json.hpp"
#include "offlinemessagemodel.hpp"
#include "redis.hpp"
#include "usermodel.hpp"
#include <functional>
#include <muduo/net/TcpConnection.h>
#include <mutex>
#include <unordered_map>

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 处理消息的回调函数类型
using MsgHandler =
    std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp time)>;

// 聊天服务器业务类 - 单例模式
// 负责处理各种业务请求，管理用户连接和消息分发
class ChatService {
  public:
    // 获取单例对象的接口函数
    static ChatService* instance();

    // 处理登录业务 - 验证用户身份并建立连接映射
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 处理退出登录业务 - 移除连接映射
    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 处理注册业务 - 创建新用户
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 一对一聊天业务 - 转发消息或存储离线消息
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 添加好友业务 - 建立好友关系
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 获取消息对应的处理器 - 根据消息类型返回对应处理函数
    MsgHandler getHandler(int msgid);

    // 服务器异常，业务重置方法 - 将所有在线用户设为离线
    void reset();

    // 处理客户端异常退出 - 清理连接资源
    void clientCloseException(const TcpConnectionPtr& conn);

    // 创建群组业务 - 创建新的聊天群
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 加入群组业务 - 将用户添加到群组
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 群组聊天业务 - 向群组内所有成员转发消息
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 从Redis消息队列中获取订阅的消息 - 用于集群环境
    // 处理来自其他服务器的消息转发
    void handleRedisSubscribeMessage(int userid, std::string msg);

  private:
    // 构造函数私有化 - 单例模式
    ChatService();

    // 存储消息ID和对应的业务处理方法
    std::unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接 - 用户ID到连接的映射
    std::unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap的线程安全
    std::mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;             // 用户数据操作
    OfflineMsgModel _offlineMsgModel; // 离线消息操作
    FriendModel _friendModel;         // 好友关系操作
    GroupModel _groupModel;           // 群组操作

    // Redis操作对象 - 用于集群环境下的消息分发
    Redis _redis;
};

#endif