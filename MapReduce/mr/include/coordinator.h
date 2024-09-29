#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include<queue>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "coordinator.grpc.pb.h"
#include "worker.grpc.pb.h"
#include <thread>
#include <atomic>

#include "config.h"

using namespace std;

//获取自己设置的rpcmodule模块，后续可以用override覆盖，因为是虚函数实现的
class Coordinator{
public:
    Coordinator(const std::vector<std::string> filenames,int nReduce);
    ~Coordinator();
    
    void WorkerRegister(shared_ptr<WorkerInfo> ctl){
        WorkerList.push_back(ctl);
        cout<<" Now you have:"<<WorkerList.size()<< "Worker"<<std::endl;
        std::cout<<WorkerList[WorkerList.size()-1]->ip<<" "<<WorkerList[WorkerList.size()-1]->port<<std::endl;
    };
    void CoordinatorServer();
    bool Done();
    void TaskDistributed();
    void CheckClientAlive();
    vector<shared_ptr<WorkerInfo>>* GetWorkerList(){
        return &WorkerList;
    }
    vector<TaskInfo>* GetMapTasks(){
        return &MapTasks;
    }
    vector<TaskInfo>* GetReduceTasks(){
        return &ReduceTasks;
    }
    void AddReduceResult(string st){
        reduceResults.push_back(st);
    }
    void ReduceResultAdd(){
        {
            std::unique_lock<std::mutex> lck(mtx);
            reducedone++;
        }
    }
    bool CheckFileServer(string ip,int port);



private:

    // void TaskDistributed();
private:
    vector<shared_ptr<WorkerInfo>> WorkerList;          //有多少个worker
    

    vector<TaskInfo> MapTasks;
    vector<TaskInfo> ReduceTasks;

    int nReduce;
    vector<string>reduceResults;
    int reducedone=0;
    mutex mtx;
};
