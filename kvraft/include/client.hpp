#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "tools.hpp"

//这一部分主要用于与KVServer进行通信的功能
//根据功能需求完成代码即可
class Clerk{
public:
    Clerk()=default;
    ~Clerk()=default;
    void Put();
    void Get();
    void Append();
    void Init();
};