#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>

// MySQL数据库操作类
// 封装了数据库连接、查询和更新等基本操作
class MySQL {
  public:
    // 初始化数据库连接
    MySQL();

    // 释放数据库连接资源
    ~MySQL();

    // 连接数据库
    // 成功返回true，失败返回false
    bool connect();

    // 更新操作
    // 用于执行insert、update、delete等SQL语句
    // sql: 要执行的SQL语句
    // 成功返回true，失败返回false
    bool update(std::string sql);

    // 查询操作
    // 用于执行select等查询SQL语句
    // sql: 要执行的SQL语句
    // 成功返回结果集指针，失败返回nullptr
    MYSQL_RES* query(std::string sql);

    // 获取数据库连接
    // 返回MySQL连接句柄
    MYSQL* getConnection();

  private:
    MYSQL* _conn; // MySQL连接句柄
};

#endif