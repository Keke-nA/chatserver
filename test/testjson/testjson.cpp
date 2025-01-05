#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

void func1()
{
    json js;
    js["msg_type"] = {1, 2, 3, 4, 5};
    js["from"] = "zhangsan";
    js["to"] = "lisi";
    js["msg"] = "hello lisi";
    string sendBuf = js.dump();
    cout << sendBuf.c_str() << endl;
}

void func2()
{
    json js;
    vector<int> tv;
    tv.push_back(1);
    tv.push_back(2);
    tv.push_back(3);
    js["list"] = tv;

    map<int, string> tm;
    tm.insert({4, "gao"});
    tm.insert({5, "xiao"});
    tm.insert({6, "yang"});
    js["path"] = tm;

    string sendBuf = js.dump();

    cout << sendBuf.c_str() << endl;
}

int main()
{
    func2();
    // func1();
    return 0;
}