#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.hpp"
#include <string>

// 群组用户类 - 继承自User类，添加了角色信息
// 复用User的基本信息，同时扩展群组相关的角色属性
class GroupUser : public User {
  public:
    // 设置用户在群组中的角色
    void setRole(std::string role) {
        this->role = role;
    }
    // 获取用户在群组中的角色
    std::string getRole() {
        return this->role;
    }

  private:
    std::string role; // 用户在群组中的角色（creator/normal）
};

#endif