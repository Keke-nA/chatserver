#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>

// 群组类 - 用于表示聊天系统中的群组信息
class Group {
  public:
    // 构造函数，初始化群组基本信息
    Group(int id = -1, std::string name = "", std::string desc = "")
        : id(id), name(name), desc(desc) {
    }

    // 设置群组ID
    void setId(int id) {
        this->id = id;
    }
    // 设置群组名称
    void setName(std::string name) {
        this->name = name;
    }
    // 设置群组描述
    void setDesc(std::string desc) {
        this->desc = desc;
    }

    // 获取群组ID
    int getId() {
        return this->id;
    }
    // 获取群组名称
    std::string getName() {
        return this->name;
    }
    // 获取群组描述
    std::string getDesc() {
        return this->desc;
    }
    // 获取群组成员列表
    std::vector<GroupUser>& getUsers() {
        return this->users;
    }

  private:
    int id;                       // 群组ID
    std::string name;             // 群组名称
    std::string desc;             // 群组描述
    std::vector<GroupUser> users; // 群组成员列表
};

#endif