#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

// 离线消息数据操作类 - 提供离线消息相关的数据库操作接口
class OfflineMsgModel {
  public:
    // 存储用户的离线消息
    // userid: 接收消息的用户ID, msg: 消息内容（JSON字符串）
    void insert(int userid, std::string msg);

    // 删除用户的所有离线消息
    // 用户登录成功后，读取完离线消息需要删除这些记录
    void remove(int userid);

    // 查询用户的所有离线消息
    // 返回存储的消息字符串列表
    std::vector<std::string> query(int userid);
};

#endif