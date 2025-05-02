#ifndef USER_H
#define USER_H

#include <string>

// User表的ORM类 - 用于表示用户信息
class User {
  public:
    // 构造函数，初始化用户基本信息
    User(
        int id = -1,
        std::string name = "",
        std::string password = "",
        std::string state = "offline") {
        this->id = id;
        this->name = name;
        this->password = password;
        this->state = state;
    }

    // 设置用户ID
    void setId(int id) {
        this->id = id;
    }
    // 设置用户名
    void setName(std::string name) {
        this->name = name;
    }
    // 设置用户密码
    void setPwd(std::string pwd) {
        this->password = pwd;
    }
    // 设置用户状态（在线/离线）
    void setState(std::string state) {
        this->state = state;
    }

    // 获取用户ID
    int getId() {
        return this->id;
    }
    // 获取用户名
    std::string getName() {
        return this->name;
    }
    // 获取用户密码
    std::string getPwd() {
        return this->password;
    }
    // 获取用户状态
    std::string getState() {
        return this->state;
    }

  private:
    int id;               // 用户ID
    std::string name;     // 用户名
    std::string password; // 用户密码
    std::string state;    // 用户状态（online/offline）
};

#endif