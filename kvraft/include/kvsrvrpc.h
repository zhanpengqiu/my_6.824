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

#include "kvsrv.grpc.pb.h"
#include "kvsrv.grpc.pb.h"
#include "client.hpp"
#include "server.hpp"
#include "msg.hpp"

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

//网络库客户端的实现
//raft server definition
class ClientServiceImpl final : public RpcModule::Rafter::Service{
    // 对服务的改写
 public:   
    ClientServiceImpl(KVServer* KVServerCtl){
        this->KVServerCtl=KVServerCtl;
    };
private:
    Status ClientRequest(ServerContext* context,const RpcModule::ClientRequestArgs* request,
                RpcModule::ClientRequestReply* reply){

        return Status::OK;
    }  
private:
    KVServer* KVServerCtl;
};


//网络库服务端
class RafterClient{
public:
    RafterClient(std::shared_ptr<Channel> channel,Clerk* ClerkCtl)
      : stub_(RpcModule::Rafter::NewStub(channel)),ClerkCtl(ClerkCtl) {}
    //设置你的样例
    int CallVoted(){
        //设置请求
        RpcModule::ClientRequestArgs args;


        RpcModule::ClientRequestReply reply;
        ClientContext context;

        //设置超时时间，报文丢弃，超时时间不超过CANDIDATE持续时间间隔
        auto deadline=chrono::system_clock::now()+chrono::milliseconds(100);
        context.set_deadline(deadline);
        Status status=stub_->ClientRequest(&context,args, &reply);

        //回复应答
        if (status.ok()) {
            return 1;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl;
            return -1;
        }
    }

private:
    std::unique_ptr<RpcModule::Rafter::Stub> stub_;
    Clerk* ClerkCtl;
};


