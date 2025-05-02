#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include <string>
#include <vector>

// 群组数据操作类 - 提供群组相关的数据库操作接口
class GroupModel {
  public:
    // 创建群组 - 向数据库中插入新群组信息
    bool creatGroup(Group& group);

    // 加入群组 - 将用户添加到指定群组
    // userid: 用户ID, groupid: 群组ID, role: 用户角色
    void addGroup(int userid, int groupid, std::string role);

    // 查询用户所在的所有群组信息
    // 返回用户加入的群组列表，包含群组信息和成员信息
    std::vector<Group> queryGroups(int userid);

    // 查询群组内其他成员ID列表
    // 主要用于群聊消息转发，获取除自己外的所有群成员
    std::vector<int> queryGroupUsers(int userid, int groupid);
};

#endif