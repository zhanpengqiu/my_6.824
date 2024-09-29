#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include "msg.h"

using namespace std;

//实现状态机
//做一个简单的存储状态机
class StateMachine{
public:
    StateMachine(){};
    ~StateMachine(){};

    void ApplyLogCommand(LogEntry entry, int logIndex);
private:
    unordered_map<string, string> state;
};

void StateMachine::ApplyLogCommand(LogEntry entry, int logIndex){
    //实现状态机的功能
    //暂时只是实现一个简单的存储
    //examples：
    //x <- 6
    //y <- 3
    //最后的状态机状态为一个map储存的：
    // {x=6, y=3}
    //简易版本：如下列所示
    string cmd=entry.m_command;
    int lastIndex=logIndex;
    int lastTerm=entry.m_term;
    //处理命令
}