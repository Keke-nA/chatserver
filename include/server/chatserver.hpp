#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
using namespace muduo;
using namespace muduo::net;

// 聊天服务器的主类
// 基于muduo网络库实现TCP服务器功能
class ChatServer {
  public:
    // 初始化聊天服务器对象
    // loop: 事件循环, listenAddr: 监听地址, nameArg: 服务器名称
    ChatServer(
        EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& nameArg);

    // 启动服务 - 开始监听并处理连接
    void start();

  private:
    // 连接回调函数 - 处理新连接建立和连接断开事件
    // 当有新客户端连接或断开时被调用
    void onConnection(const TcpConnectionPtr& conn);

    // 消息回调函数 - 处理收到的消息
    // 当收到客户端消息时被调用
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    TcpServer _server; // muduo库的TCP服务器对象
    EventLoop* _loop;  // 事件循环对象指针
};

#endif