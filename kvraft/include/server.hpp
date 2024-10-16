#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "tools.hpp"
#include "raft.h"

//这一部分完成的是KVserver的交流
//他位于raft的上一层。他可以知道当前的server是不是leader
//这里的rafter还需要进行apply的改进
//满足kv的需求，能对特定键值进行操作

class KVServer{
public:
    KVServer(int id);
    ~KVServer()=default;
    void Put(const std::string& key, const std::string& value);
    std::string Get(const std::string& key);
    void Append();
    void Init();
private:
    shared_ptr<Raft> raft;          //这是我的raft实现
};