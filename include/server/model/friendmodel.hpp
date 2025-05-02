#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"
#include <vector>

// 好友关系数据操作类 - 提供好友关系相关的数据库操作接口
class FriendModel {
  public:
    // 添加好友关系 - 在Friend表中插入好友关系记录
    // userid: 用户ID, friendid: 好友ID
    void insert(int userid, int friendid);

    // 查询用户的好友列表
    // 返回用户的所有好友信息，包括ID、名称和在线状态
    std::vector<User> query(int userid);
};

#endif