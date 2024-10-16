#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/health_check_service_interface.h>

#include "raftrpc.grpc.pb.h"
#include "raftrpc.grpc.pb.h"
#include "raft.h"
#include "msg.h"
using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

//网络库客户端
//raft server definition
class ClientServiceImpl final : public RpcModule::Rafter::Service{
    // 对服务的改写
 public:   
    ClientServiceImpl(Raft* RaftCtl){
        this->RaftCtl=RaftCtl;
    };
private:
    Status RequestVote(ServerContext* context,const RpcModule::RequestVoteRequest* request,
                RpcModule::RequestVoteReply* reply){
        //实现请求投票的函数
        //1. 获取基本参数
        // RaftCtl->RandomSleep();   
        int tmp=0+rand()%10;
        if(tmp>4)return Status::CANCELLED;    
         
        int term=request->term();
        int candidateId=request->candidateid();
        int lastLogTerm=request->lastlogterm();
        int lastLogIndex=request->lastlogindex();

        //2. 受到此消息的服务器实现请求投票的功能
        //2.1. 验证任期
        if(term>=RaftCtl->GetTerm()){
            if(term>RaftCtl->GetTerm()){                //因为发现别人的任期比自己大
                RaftCtl->SetVoteFor(-1);                //重置此轮中这个服务器的投票对象  
                RaftCtl->SetTerm(term);                 //跟随他人的任期
                RaftCtl->setRafterType(FOLLOWER); 
            }
            // cout<<lastLogIndex<<" "<<RaftCtl->GetLastIndex()<<" "<<lastLogTerm<<" "<<RaftCtl->GetLastLogTerm()<<endl;
            //投票条件
            if(RaftCtl->GetVoteFor()==-1||RaftCtl->GetVoteFor()==candidateId){
                //日志一致性，检查传来的日志是不是最新的,比自己新，就给出投票
                if(lastLogIndex>=RaftCtl->GetLastIndex()&&lastLogTerm>=RaftCtl->GetLastLogTerm()){
                    // cout<<"Node "<<RaftCtl->GetMyId()<<"vote for "<<candidateId<<endl;
                    reply->set_term(RaftCtl->GetTerm());
                    reply->set_votegranted(true);               //同意投票
                    RaftCtl->setRafterType(FOLLOWER);           //成为跟随者，用于LEADER转变成FOLLOWER
                    RaftCtl->SetFollowerFlag(true);
                    RaftCtl->SetVoteFor(candidateId);
                    cout<<"Term "<<term<<" node "<<RaftCtl->GetMyId()<<" been voted for "<<RaftCtl->GetVoteFor()<<" candidateid "<<candidateId<<endl;
                    return Status::OK;
                }
            }
        }
        //其余情况，不给予投票，并且返回自己的term，用于给其他的服务器更新任期
        reply->set_term(RaftCtl->GetTerm());
        reply->set_votegranted(false);
        return Status::OK;
    }  
    Status Appendntries(ServerContext* context,const RpcModule::AppendntriesRequest* request,
                RpcModule::AppendntriesReply* reply){
        //接受leader传输过来的消息
        // RaftCtl->RandomSleep();    
        // //随机不正常返回
        //如果在安装snapshot的时候，就不接受appendtries的调用，直接推出
        if(!RaftCtl->IsLogReciveFlagAviailable())return Status::CANCELLED;

        if(RaftCtl->GetMyId()==2||RaftCtl->GetMyId()==3){
            int tmp=0+rand()%10;
            if(tmp>3)return Status::CANCELLED;
        }
        
        int term=request->term();
        int leaderId=request->leaderid();
        int prevLogIndex=request->prevlogindex();
        int prevLogTerm=request->prevlogterm();
        int commitIndex=request->commitindex();
        
        vector<LogEntry>entries;
        for (int i = 0; i < request->entries_size(); ++i) {
            LogEntry entry(request->entries(i).cmd(), request->entries(i).term(), request->entries(i).index());
            entries.push_back(entry);
        }
        cout<<"Node "<<RaftCtl->GetMyId()<<" Term "<<RaftCtl->GetTerm()<<" commitindex "<<RaftCtl->GetCommitIndex()<<" LastIndex"<<RaftCtl->GetLastIndex()<<" LastTerm "<<RaftCtl->GetLastLogTerm()<<" received: "<<"Term "<<term<<" LeaderId "<<leaderId<<" PrevLogIndex "<<prevLogIndex<<" PrevLogTerm "<<prevLogTerm<<" CommitIndex "<<commitIndex<<" Entries size "<<entries.size()<<endl;

        int my_lastindex=RaftCtl->GetLastIndex();
        int my_lastterm=RaftCtl->GetLastLogTerm();

        //发现如果自己的日志明显低于主机的快照，那么告知主机，我需要你发送快照文件，让我更新我自己
        reply->set_installsnapshot(false);
        if(prevLogIndex-my_lastindex>20){
            reply->set_installsnapshot(true);
        }
        //处理消息
        if(term>=RaftCtl->GetTerm()){
            RaftCtl->SetLeaderId(leaderId);
            if(term>RaftCtl->GetTerm()){
                //确认主从位置
                RaftCtl->SetTerm(term);
                RaftCtl->SetVoteFor(leaderId);           //重置此轮中这个服务器的投票对象为leader，并转换成follower状态     
                RaftCtl->setRafterType(FOLLOWER);        //转换成follower状态
            }
            //只判断成功的情况
            if(entries.size()!=0){//如果有新传入的日志
                if(my_lastindex > prevLogIndex){
                    // 丢弃不匹配的日志,回退到prevlog指示的位置
                    RaftCtl->RollLog(prevLogIndex);
                    my_lastindex=RaftCtl->GetLastIndex();
                    my_lastterm=RaftCtl->GetLastLogTerm();
                }
                //找到日志匹配的点，找到之后就把新的log增加到主机当中
                if(my_lastindex == prevLogIndex&&my_lastterm==prevLogTerm){
                    //增加entories到raft的日志当中
                    for (LogEntry entry : entries) {
                        RaftCtl->AddLog(entry);
                    }
                    //告知，此时有leader来的消息，更新自己的定时器
                    RaftCtl->SetFollowerFlag(true);

                    reply->set_term(RaftCtl->GetTerm());
                    reply->set_success(true);
                    return Status::OK;
                } 
            }
            else{//如果日志为空，那么要么是commit一个日志index，要么就是确认领导权，在这一部分只要check commitlog就行。
            //还有一种情况，就是传入的日志虽然为空，但是还是有可能会进行他哦干部日志的工作
                cout<<"prevLogIndex "<<prevLogIndex<<" my_lastindex "<<my_lastindex<<" prevLogTerm "<<prevLogTerm<<" RaftCtl->GetIndexTerm(my_last_term) "<<RaftCtl->GetIndexTerm(my_lastindex);
                if(prevLogIndex>my_lastindex||prevLogTerm>RaftCtl->GetIndexTerm(prevLogIndex)){//进行日志匹配工作
                    RaftCtl->SetFollowerFlag(true);

                    reply->set_term(RaftCtl->GetTerm());
                    reply->set_success(false);
                    return Status::OK;
                }
                if(prevLogIndex!=-1&&(prevLogTerm!=RaftCtl->GetIndexTerm(prevLogIndex))){
                    RaftCtl->SetFollowerFlag(true);

                    reply->set_term(RaftCtl->GetTerm());
                    reply->set_success(false);
                    return Status::OK;
                }
                //commit通过leader给他更新，当leadercommit之后
                RaftCtl->SetCommitIndex(max(RaftCtl->GetCommitIndex(),commitIndex));
                //告知，此时有leader来的消息，更新自己的定时器
                RaftCtl->SetFollowerFlag(true);
                reply->set_term(RaftCtl->GetTerm());
                reply->set_success(true);
                return Status::OK;
            }
        }
        //当别人的term小于本机的情况，就返回自己的term给他跟上就行。这个success参数设不设置都没问题
        reply->set_term(RaftCtl->GetTerm());
        reply->set_success(false);
        return Status::OK;
    }

    Status InstallSnapshot(ServerContext* context,const RpcModule::InstallSnapshotRequest* request,
                RpcModule::InstallSnapshotReply* reply){
        //或许传入的参数
        int term=request->term();
        int last_include_index=request->lastincludeindex();
        int last_include_term=request->lastincludeterm();
        int leaderid=request->leaderid();
        int index=request->leaderid();

        shared_ptr<ApplyMsg> snapshotMsg=make_shared<ApplyMsg>();
        snapshotMsg->Snapshot=request->statemachine(0).data();
        snapshotMsg->SnapshotTerm=request->statemachine(0).term();
        snapshotMsg->SnapshotIndex=request->statemachine(0).index();

        vector<LogEntry>entries;
        for (int i = 0; i < request->entries_size(); ++i) {
            LogEntry entry(request->entries(i).cmd(), request->entries(i).term(), request->entries(i).index());
            entries.push_back(entry);
        }
        //做出响应
        //首先需要确认是不是适合应用到这台主机上面
        if(term>=RaftCtl->GetTerm()){
            RaftCtl->SetLeaderId(leaderid);
            if(term>RaftCtl->GetTerm()){
                //确认主从位置
                RaftCtl->SetTerm(term);
                RaftCtl->SetVoteFor(leaderid);           //重置此轮中这个服务器的投票对象为leader，并转换成follower状态     
                RaftCtl->setRafterType(FOLLOWER);        //转换成follower状态
            }
            //然后开始处理业务逻辑
            //1.更新当前的状态机
            RaftCtl->LockLogReciveFlag();
            RaftCtl->state_machine->UpdateSnapshot(snapshotMsg);
            RaftCtl->state_machine->UpdateApplyMsg(snapshotMsg);
            RaftCtl->state_machine->SaveSnapshot(RaftCtl->GetMyId());
            RaftCtl->DelLogFile();
            //2.在这里原本的逻辑是清除commitindex之前的日志，但是我这里为了方便全部清空
            RaftCtl->RollLog(RaftCtl->state_machine->GetIndex()-1);
            //3. 然后加入新的日志
            RaftCtl->AddLogList(entries);
            //4.设置commitindex
            RaftCtl->SetLastCommitIndex(request->statemachine(0).index());
            RaftCtl->SetCommitIndex(last_include_index);
            RaftCtl->SetLastAppliedLogIndex(request->statemachine(0).index());
            RaftCtl->UnlockLogReciveFlag();
            cout<<endl<<"InstallsnapShot last_include_index: "<<last_include_index<<" CommitIndex: "<<RaftCtl->GetCommitIndex()<<" log_size "<<entries.size()<<endl;
        }
        reply->set_term(RaftCtl->GetTerm());
        return Status::OK;
    }
private:
    Raft* RaftCtl;
};


//网络库服务端
class RafterClient{
public:
    RafterClient(std::shared_ptr<Channel> channel,Raft* RaftCtl)
      : stub_(RpcModule::Rafter::NewStub(channel)),RaftCtl(RaftCtl) {}
    //设置你的样例
    int CallVoted(){
        //设置请求
        RpcModule::RequestVoteRequest args;
        args.set_term(RaftCtl->GetTerm());
        args.set_candidateid(RaftCtl->GetMyId());
        args.set_lastlogindex(RaftCtl->GetLastIndex());
        args.set_lastlogterm(RaftCtl->GetLastLogTerm());

        RpcModule::RequestVoteReply reply;
        ClientContext context;

        //设置超时时间，报文丢弃，超时时间不超过CANDIDATE持续时间间隔
        auto deadline=chrono::system_clock::now()+chrono::milliseconds(100);
        context.set_deadline(deadline);
        Status status=stub_->RequestVote(&context,args, &reply);

        //回复应答
        if (status.ok()) {
            //如果成功获取则获取相应的应答参数
            if(reply.term()>RaftCtl->GetTerm()){
                RaftCtl->SetTerm(reply.term());
                RaftCtl->SetVoteFor(-1);                 //清空旧的投票
                RaftCtl->setRafterType(FOLLOWER);       //回归追随者状态
            }
            else if(reply.term()==RaftCtl->GetTerm()){
                if(reply.votegranted()){
                    RaftCtl->AddVotesReceived();        //增加
                }
            }
            //不会出现比自己任期小的情况出现，因为如果出现，就已经在rpccall的时候，客户端自动处理了
            return 1;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl;
            return -1;
        }
    }
    
    int CallAppendEntries(int id){
        RpcModule::AppendntriesRequest args;
        if(RaftCtl->GetPreLogIndex(id)<RaftCtl->state_machine->GetIndex()-1)return CallInstallSnapShot(id);
        //初始化参数
        args.set_term(RaftCtl->GetTerm());
        args.set_leaderid(RaftCtl->GetMyId());
        args.set_prevlogindex(RaftCtl->GetPreLogIndex(id));
        args.set_prevlogterm(RaftCtl->GetPreLogTerm(id));
        args.set_commitindex(RaftCtl->GetCommitIndex());

        cout<<"asdasdasdasdqwexcgjihdb"<<RaftCtl->state_machine->GetIndex()<<" prelogindex "<<RaftCtl->GetPreLogIndex(id)<<" PreLogTerm: "<<RaftCtl->GetPreLogTerm(id)<<endl;

        auto entries=RaftCtl->GetEntries(RaftCtl->GetPreLogIndex(id)+1);
        for(auto iter=entries.begin(); iter!=entries.end(); iter++){
            RpcModule::Entry* entry=args.add_entries();
            entry->set_term(iter->m_term);
            entry->set_cmd(iter->m_command);
            entry->set_index(iter->m_index);
        }
        int entries_size=entries.size();
        RpcModule::AppendntriesReply reply;
        ClientContext context;
        //append不需要设置超时时间
        Status status=stub_->Appendntries(&context,args, &reply);

        //得到回复
        //todo add matchindex
        if (status.ok()) {
            //如果成功获取则获取相应的应答参数
            if(reply.term()>RaftCtl->GetTerm()){
                RaftCtl->SetTerm(reply.term());
                RaftCtl->SetVoteFor(-1);                 //清空旧的投票
                RaftCtl->setRafterType(FOLLOWER);       //回归追随者状态
            }else if(reply.term()==RaftCtl->GetTerm()){
                if(reply.success()){
                    RaftCtl->SetPreLogIndex(id,entries_size);       //设置nextindex的
                    RaftCtl->SetMatchIndex(id,RaftCtl->GetPreLogIndex(id));        //设置matchindex的
                    cout<<" appendiex success : prelog&id"<<RaftCtl->GetPreLogIndex(id)<<" "<<id<<" matchindex "<<RaftCtl->GetMatchIndex(id)<<endl;
                }else{
                    if(!reply.installsnapshot()) 
                        RaftCtl->SetPreLogIndex(id,-1);         //设置nextindex的
                }
            }
            if(reply.installsnapshot()==true){
                CallInstallSnapShot(id);
            }
            return 1;
        } else {
            // std::cout << status.error_code() << ": " << status.error_message()
            //             << std::endl;
            return -1;
        }

    }
    int CallInstallSnapShot(int id){//这一部分要设置以下
        //设置参数
        RpcModule::InstallSnapshotRequest args;
        args.set_term(RaftCtl->GetTerm());
        args.set_leaderid(RaftCtl->GetMyId());
        args.set_lastincludeindex(RaftCtl->GetCommitIndex());
        args.set_lastincludeterm(RaftCtl->GetIndexTerm(RaftCtl->GetCommitIndex()));

        shared_ptr<ApplyMsg> snapshotMsg=make_shared<ApplyMsg>();
        RaftCtl->state_machine->GetSnapShotMsg(snapshotMsg);

        RpcModule::StateMachine* stateMachineMsg=args.add_statemachine();
        stateMachineMsg->set_index(snapshotMsg->SnapshotIndex);
        stateMachineMsg->set_data(snapshotMsg->Snapshot);
        stateMachineMsg->set_term(snapshotMsg->SnapshotTerm);

        //获取commit之后的日志
        auto entries=RaftCtl->GetEntries(RaftCtl->state_machine->GetIndex(),RaftCtl->GetCommitIndex()+1);
        for(auto iter=entries.begin(); iter!=entries.end(); iter++){
            RpcModule::Entry* entry=args.add_entries();
            entry->set_term(iter->m_term);
            entry->set_cmd(iter->m_command);
            entry->set_index(iter->m_index);
        }

        RpcModule::InstallSnapshotReply reply;
        ClientContext context;
        //发出命令
        Status status=stub_->InstallSnapshot(&context,args, &reply);
        //得到回复
        if (status.ok()) {
            //如果成功获取则获取相应的应答参数
            if(reply.term()>RaftCtl->GetTerm()){
                RaftCtl->SetTerm(reply.term());
                RaftCtl->SetVoteFor(-1);                 //清空旧的投票
                RaftCtl->setRafterType(FOLLOWER);       //回归追随者状态
            }
            
            return 1;
        }else {
            // std::cout << status.error_code() << ": " << status.error_message()
            //             << std::endl;
            return -1;
        }
    }

private:
    std::unique_ptr<RpcModule::Rafter::Stub> stub_;
    Raft* RaftCtl;
};


