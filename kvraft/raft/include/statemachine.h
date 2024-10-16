#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "msg.h"

using namespace std;

//实现状态机
//做一个简单的存储状态机
class StateMachine{
public:
    StateMachine(){
        lastIndex=-1;
        lastTerm=-1;
        cmd="";

        snapindex=-1;
        snapterm=-1;
        snapcmd="";
    };
    ~StateMachine(){};

    void ApplyLogCommand(LogEntry entry, int logIndex){
        //这里是实现状态机apply的功能，暂时这里保存执行的最后一条命令
        unique_lock<mutex> lock(mtx);
        cmd=entry.m_command;

        lastIndex=logIndex;
        lastTerm=entry.m_term;
        cout<<"Applying a command: "<<cmd<<endl;
    }
    void GetApplyMsg(shared_ptr<ApplyMsg> entry){
        unique_lock<mutex> lock(mtx);
        entry->Snapshot=cmd;
        entry->SnapshotIndex=lastIndex;
        entry->SnapshotTerm=lastTerm;
    }
    void GetSnapShotMsg(shared_ptr<ApplyMsg> entry){
        unique_lock<mutex> lock(mtx);
        entry->Snapshot=snapcmd;
        entry->SnapshotIndex=snapindex;
        entry->SnapshotTerm=snapterm;
    }
    int GetIndex(){
        unique_lock<mutex> lock(mtx);
        return snapindex+1;
    }
    int GetLastTerm(){
        unique_lock<mutex> lock(mtx);
        return snapterm;
    }
    bool IsStartMakeSnapshot(){
        unique_lock<mutex> lock(mtx);
        return lastIndex-snapindex>10?true:false;
    }
    void UpdateSnapshot(shared_ptr<ApplyMsg> snapshotMsg){
        unique_lock<mutex> lock(mtx);
        snapcmd=snapshotMsg->Snapshot;
        snapindex=snapshotMsg->SnapshotIndex;
        snapterm=snapshotMsg->SnapshotTerm;
    }
    void UpdateApplyMsg(shared_ptr<ApplyMsg> applyMsg){
        unique_lock<mutex> lock(mtx);
        cmd=applyMsg->Snapshot;
        lastIndex=applyMsg->SnapshotIndex;
        lastTerm=applyMsg->SnapshotTerm;
    }
    void SaveSnapshot(int id){
        //保存状态机
        shared_ptr<ofstream>file;
        string SnapShotPath="/home/qzp/lab/study/src/kvraft/raft/Snapshot_Node"+to_string(id)+".txt";
        file =make_shared<ofstream>(SnapShotPath);
        // 检查文件是否成功打开
        if (!file->is_open()) {
                std::cerr << "无法打开文件 " << SnapShotPath << " 进行写入。\n";
                return;
        }
        *file<<snapindex<<endl;
        *file<<snapterm<<endl;
        *file<<snapcmd<<endl;
        
        file->close();
    
    }
    void LoadSnapshot(){

    }
private:
    // 状态机存储
    mutex mtx;
    int lastIndex;
    int lastTerm;
    string cmd;
    string snapcmd;
    int snapindex;
    int snapterm;

    vector<string> total_cmd;
};