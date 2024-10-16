#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <unistd.h>

#define RAFTSERVERNUM 5

#include "statemachine.h"
using namespace std;

class Raft{
public: 
    Raft(int me);
    ~Raft();
    State GetState();
    void LogPersist();
    void ReadServerConfig(string filepath);
    vector<uint8_t> ReadPersist();
    void Snapshot(int index,vector<uint8_t>& out);
    void SendRequestVote();                 //发送投票请求
    void SendAppendEntries();
    
    void ClientServer();

    StartMsg Start();
    void Kill();
    bool Killed();
    void ticker();
    void Init();
    void Run();
    void PrintLog();
    void CheckCommitIndex();
    //持久化log到状态机上（这里就暂且保存在硬盘上）
    void ApplyLogLoop();

    //模拟日志随机增加的效果，如果当前主机是leader，那么每隔一段时间都会增加1-5个日志到log中去。
    //这个用一个线程完成，每隔1-5s，都会有一个日志输入。
    void RandomAddLog();

    void MakeSnapShot();

    // request voted的时候获取一些必要的参数的函数

    void RandomSleep(){
        int time=0+(rand()%(100));
        if(time>90)sleep(1);
    }

    int GetTerm(){
        return currentTerm;
    }
    void SetTerm(int term){
        currentTerm=term;
    }
    int GetMyId(){
        return me;
    }
    int GetLastIndex(){
        unique_lock<mutex>lock(log_mtx);
        if(log.size()==0){
            if(state_machine->GetIndex()==0){
                return -1;
            }else{
                return state_machine->GetIndex()-1;
            }
        }
        return log[log.size()-1].m_index;
    }
    int GetLastLogTerm(){
        unique_lock<mutex>lock(log_mtx);
        if(log.size()==0){
            if(state_machine->GetIndex()==0){
                return -1;
            }else{
                return state_machine->GetLastTerm();
            }
        }
        return log[log.size()-1].m_term;
    }
    int GetIndexTerm(int index){
        int index_log=index-state_machine->GetIndex();
        unique_lock<mutex>lock(log_mtx);
        if(log.size()==0){
            return state_machine->GetLastTerm();
        }else{
            if(index_log<0)return state_machine->GetLastTerm();
            if(log.size()-1<index_log)return -1;
            else return log[index_log].m_term;
        }
    }
    void SetVoteFor(const int id){
        unique_lock<mutex> lock(votedFor_mtx);
        votedFor=id;
    }
    int GetVoteFor(){
        unique_lock<mutex> lock(votedFor_mtx);
        return votedFor;
    }
    bool GetVotesReceivedIsMajority(){
        unique_lock<mutex> lock(mtx);
        if(votesReceived>=majorityVotes)return true;
        return false;
    }
    void AddVotesReceived(){
        unique_lock<mutex> lock(mtx);
        votesReceived++;
    }
    void ResetVotesReceived(){
        unique_lock<mutex> lock(mtx);
        votesReceived=0;
    }
    void SubVotesReceived(){
        unique_lock<mutex> lock(mtx);
        votesReceived--;
    }
    int GetPreLogIndex(int id){
        return nextIndex[id]-1;
    }
    void SetPreLogIndex(int id,int addsize){
        //这个size代表了你传输的entries尺寸
        nextIndex[id] += addsize;        
    }
    int GetMatchIndex(int id){
        unique_lock<mutex> lock(matchindex_mtx);
        return matchIndex[id];
    }
    void SetMatchIndex(int id,int size){
        unique_lock<mutex> lock(matchindex_mtx);
        matchIndex[id]=size;
    }
    int GetPreLogTerm(int id){
        if(GetPreLogIndex(id)<0)return -1;
        unique_lock<mutex> lock(mtx);
        int index;
        if(state_machine->GetIndex()==0) index=GetPreLogIndex(id);
        else  index=GetPreLogIndex(id)-state_machine->GetIndex();//剪掉之后就是大小，还要再剪掉一个1代表index
        {
            unique_lock<mutex>lock(log_mtx);
            //如果想获取的index小于当前日志的最小位置，那么返回状态机的日志
            if(log.size()>0){
                if(GetPreLogIndex(id)<log[0].m_index){
                    return state_machine->GetLastTerm();
                }
            }else{
                return state_machine->GetLastTerm();
            }
        }
        return log[index].m_term;
    }
    int GetCommitIndex(){
        unique_lock<mutex>lock(commit_mtx);
        return commitIndex;
    }
    void SetLastCommitIndex(int index){
        last_commitindex=index;
    }
    void SetCommitIndex(int index){
        unique_lock<mutex>lock(commit_mtx);
        int tmp=commitIndex;
        commitIndex=index;
        if(index!=tmp){
            unique_lock<mutex>lock(cv_commit_mtx);
            cv_commit.notify_one();
        }
    }
    std::vector<LogEntry> GetEntries(int index){//获取对应长度的log
        int index_log=index-state_machine->GetIndex();
        unique_lock<mutex> lock(log_mtx);
        vector<LogEntry> outentries;
        if(index_log>log.size()||index_log<0) return outentries;
        for(auto entry=log.begin()+index_log; entry!=log.end();entry++){
            outentries.push_back(*entry);
        }
        return outentries;
    }
    std::vector<LogEntry> GetEntries(int index,int index_end){//获取对应长度的log,重载
        int index_log=index-state_machine->GetIndex();
        int index_log_end=index_end-state_machine->GetIndex();
        unique_lock<mutex> lock(log_mtx);
        vector<LogEntry> outentries; 
        if(index_log>index_log_end)return outentries;
        for(auto entry=log.begin()+index_log; entry!=log.begin()+index_log_end&&entry!=log.end();entry++){
            outentries.push_back(*entry);
        }
        return outentries;
    }
    void RollLog(const int index){
        int size=index+1-state_machine->GetIndex();
        unique_lock<mutex> lock(mtx);
        while(log.size()>0&&log.size()!=size){
            log.pop_back();
        }      
    }
    void AddLog(LogEntry entry){
        unique_lock<mutex> lock(mtx);
        log.push_back(entry);
    }
    void AddLogList(vector<LogEntry> entries){
        unique_lock<mutex> lock(mtx);
        for(auto entry:entries){
            log.push_back(entry);
        }
    }
    //获取appendenties的时候获取必要参数的函数
    bool GetRafterTypeChangedFlag(){
        unique_lock<mutex> lock(RafterTypeChangedFlag_mutex);
        return RafterTypeChangedFlag;
    }
    void SetRafterTypeChangedFlag(bool flag){
        unique_lock<mutex> lock(RafterTypeChangedFlag_mutex);
        RafterTypeChangedFlag=flag;
    }

    //简单实现
    RafterType GetRafterType(){
        unique_lock<mutex> lock(raftertypechanged_mutex);
        return raftertype;
    }
    void setRafterType(RafterType type){
        //如果当前被设置为CANDIDATE就代表目前
        if(type == CANDIDATE){
            currentTerm++;
            SetVoteFor(me);
        }
        else if (type == LEADER){               //初始化当前的nextindex与matchindex
            int lastindex=GetLastIndex();
            for(int i=0;i<RAFTSERVERNUM;i++){
                nextIndex[i]=lastindex+1;
                //一开始matchindex是等于零，当匹配到对应的日志的时候，从nextindex的前一个下标开始
                unique_lock<mutex> lock(matchindex_mtx);
                matchIndex[i]=-1;
            }
            UpdateMyMatchIndex();//更新自己的matchindex为log的最后一位
        }
        {
            unique_lock<mutex> lock(raftertypechanged_mutex);
            raftertype=type;
        }
        SetRafterTypeChangedFlag(true);
        //告诉定时器，有身份转化的信号传输过来了
    }
    void UpdateMyMatchIndex(){
        unique_lock<mutex> lock(matchindex_mtx);
        matchIndex[me]=GetLastIndex();
    }
    bool GetFollowerFlag(){
        return followerflag;
    }
    void SetFollowerFlag(bool flag){
        followerflag=flag;
    }
    int GetLeaderId(){
        return leaderId;
    }
    void SetLeaderId(int id){
        leaderId=id;
    }
    bool IsLogReciveFlagAviailable(){
        unique_lock<mutex> lock(log_recive_flag_mutex);
        return log_recive_flag;
    }
    void LockLogReciveFlag(){
        unique_lock<mutex> lock(log_recive_flag_mutex);
        log_recive_flag=false;
    }
    void UnlockLogReciveFlag(){
        unique_lock<mutex> lock(log_recive_flag_mutex);
        log_recive_flag=true;
    }
    void SetLastAppliedLogIndex(int index){
        lastApplied=index;
    }
    void DelLogFile(){
        std::ofstream fileStream(logFilePath, std::ios::out | std::ios::trunc);

        // 检查文件是否成功打开
        if (!fileStream.is_open()) {
            std::cerr << "Failed to open file." << std::endl;
        }
        // 关闭文件流
        fileStream.close();

        std::cout << "File cleared successfully." << std::endl;
    }

private:
    bool followerflag=true;
    unordered_map<int,NetMsg> id_netmsg;
    void* timer;                        //定时器
    mutex RafterTypeChangedFlag_mutex;
    bool RafterTypeChangedFlag=true;

    std::atomic<uint32_t> dead;
    int me;
    int leaderId=-1;
    RafterType raftertype;
    int  votesReceived;
    int majorityVotes;
    
    //所有服务器上的持久化状态
    int currentTerm;            //服务器看到的最新任期，首次启动为0
    int votedFor;                  //votedFor为-1的时候代表他设都没有投票
    mutex votedFor_mtx;
    //当前日至记录
    mutex log_mtx;
    std::vector<LogEntry>log;       //每个日志条目都包含状态机命令，以及领导者和收到词条目任期（第一个索引为1，所以在开始push了一个空的日志进去）
    
    //所有服务易失状态
    int commitIndex;            //一直已提交最高日志条目索引（初始为0单调递增）
    int lastApplied;            //已应用到状态机最高日志条目索引

    //领导者上的易失状态
    vector<int> nextIndex;              //对于每个服务器，领导者要给次服务器发送的下一个日志条目的索引
    //lastLogIndex：表示当前节点日志的最后一条条目的索引。
    //matchIndex[i]：表示节点i（通常是Follower）与Leader之间最后一条匹配的日志条目的索引。
    mutex matchindex_mtx;
    vector<int> matchIndex;             // 对于每个服务器，已知在此服务器上复制的最高日志条目索引
    mutex mtx;
    mutex raftertypechanged_mutex;

    //告诉别人，我的commitindex有更新，此时就可以持久化log到硬盘上
    condition_variable cv_commit;
    mutex cv_commit_mtx;

    //log是否可以接受客户端的log
    mutex log_recive_flag_mutex;
    bool log_recive_flag=true;

    mutex commit_mtx;
    mutex apply_mtx;

    //持久化日志到硬盘上
    int last_commitindex=-1;

    //状态机
public:
    shared_ptr<StateMachine> state_machine;
    string logFilePath;
};