#pragma once

#include <thread>
#include <functional>
#include "raft.h"

/// @brief ：这是用于raft定时管理的功能，会根据raft的状态执行相应的函数
class Timer{
public:
    Timer(Raft* RaftCtl):RaftCtl(RaftCtl){
        //启动一个线程循环执行定时器

    };
    ~Timer()=default;

    void TickerLoop();

    void LeaderLoop();
    void FollowerLoop();
    void CandidateLoop();

    std::chrono::milliseconds GenerateRandomTime(int minTime,int maxTime){
        if (minTime > maxTime) {  
            std::swap(minTime, maxTime);  
        }  
        int time=minTime+(rand()%(maxTime-minTime+1));
        return std::chrono::milliseconds(time);;
    }

    void usleepfor20ms(){
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
private:   
    Raft* RaftCtl;
};

void Timer::LeaderLoop(){
    /// @brief Leader 每隔80ms发送一次心跳包
    auto start_time = std::chrono::steady_clock::now();
    //执行leader的操作
    RaftCtl->SendAppendEntries();
    auto timeSpilt=GenerateRandomTime(30,50);
    // int time=0+(rand()%(100));
    // if(time>75)sleep(1);
    //在每个时间片检查是否有身份转变
    while(std::chrono::steady_clock::now()-start_time <= std::chrono::milliseconds(timeSpilt)){
        usleepfor20ms();
        //检查当前是不是有身份的变化，有的话就退出当前的循环
        if(RaftCtl->GetRafterTypeChangedFlag())return;
    }
}
void Timer::CandidateLoop(){
    /// @brief Candidate 每次发送requestVote的时候就
    auto start_time = std::chrono::steady_clock::now();
    // 执行follower的操作
    RaftCtl->SendRequestVote();
    auto timeSpilt=GenerateRandomTime(150,300);
    while(std::chrono::steady_clock::now()-start_time <= std::chrono::milliseconds(timeSpilt)){
        usleepfor20ms();
        //检查当前是不是有身份的变化，有的话就退出当前的循环。
        if(RaftCtl->GetRafterTypeChangedFlag())return;
    }
    //todo 回到follower
    RaftCtl->setRafterType(FOLLOWER);
}
void Timer::FollowerLoop(){
    /// @brief Candidate 每次发送requestVote的时候就
    auto start_time = std::chrono::steady_clock::now();
    RaftCtl->SetFollowerFlag(false);
    auto timeSpilt=GenerateRandomTime(150,300);
    while(std::chrono::steady_clock::now()-start_time <= std::chrono::milliseconds(timeSpilt)){
        usleepfor20ms();
        //检查是否有Appendentries事件到来,如果有，则刷新并返回
        if(RaftCtl->GetFollowerFlag())return ;
    }
    //超时回到CANDIDATE
    int id=RaftCtl->GetMyId();
    cout<<"Node "<<id<<" be a CANDIDATE"<<endl;
    RaftCtl->setRafterType(CANDIDATE);
}

void Timer::TickerLoop(){
    while(1){
        int id=RaftCtl->GetMyId();
        //todo 首先检查是否需要转换状态
        switch(RaftCtl->GetRafterType()){
            case LEADER:{
                cout<<"Node "<<id<<" is LEADER"<<endl;
                LeaderLoop();
                break;
            }
            case FOLLOWER:{
                cout<<"Node "<<id<<" is FOLLOWER"<<endl;
                FollowerLoop();
                break;
            }
            case CANDIDATE:{
                cout<<"Node "<<id<<" is CANDIDATE"<<endl;
                CandidateLoop();
                break;
            }
            default:
            break;
        }
    }
}