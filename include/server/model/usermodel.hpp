#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// 用户数据操作类 - 提供用户相关的数据库操作接口
class UserModel {
  public:
    // 新增用户 - 向User表中插入新用户记录
    // 成功返回true并设置user的id字段
    bool insert(User& user);

    // 根据ID查询用户信息
    // 返回包含用户完整信息的User对象
    User query(int id);

    // 更新用户状态 - 修改用户的在线/离线状态
    // 用于用户登录、注销时更新状态
    bool updateState(User user);

    // 重置所有在线用户的状态
    // 用于服务器异常退出时，将所有在线用户状态设为离线
    void resetState();
};

#endif