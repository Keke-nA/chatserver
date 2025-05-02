#ifndef REDIS_H
#define REDIS_H

#include <functional>
#include <hiredis/hiredis.h>
#include <string>

// Redis操作类
// 负责与Redis服务器通信，实现基于发布-订阅模式的跨服务器通信
// 用于集群环境下的消息分发
class Redis {
  public:
    Redis();
    ~Redis();

    // 连接Redis服务器
    // 初始化publish和subscribe两个上下文连接
    bool connect();

    // 向指定通道发布消息
    // channel: 通道ID, message: 消息内容
    bool publish(int channel, std::string message);

    // 订阅指定通道的消息
    // 开始接收channel通道的消息
    bool subscribe(int channel);

    // 取消订阅指定通道
    // 停止接收channel通道的消息
    bool unsubscribe(int channel);

    // 在独立线程中监听订阅通道的消息
    // 接收到消息后通过回调函数处理
    void observer_channel_message();

    // 设置消息处理的回调函数
    // 当收到订阅消息时，调用该函数处理
    void init_notify_handler(std::function<void(int, std::string)> fn);

  private:
    redisContext* _publish_context;  // 发布消息的上下文连接
    redisContext* _subcribe_context; // 订阅消息的上下文连接
    std::function<void(int, std::string)>
        _notify_message_handler; // 消息处理回调函数
};

#endif
