#include "coordinator.h"

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
//获取几个rpc的模块
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;
using grpc::ClientWriter;
using namespace std;

// rpc server defintion
class ServerServiceImpl final : public RpcModule::Coordinator::Service{
    // 对服务的改写
 public:   
    ServerServiceImpl(Coordinator* CoorCtl){
        this->CoorCtl=CoorCtl;
    };
private:
    Status Example(ServerContext* context, const RpcModule::ExampleArgs* request,
                RpcModule::ExampleReply* reply) override{
        int tmp=request->x();
        reply->set_y(tmp+1);
        return Status::OK;
    }

    Status Register(ServerContext* context, const RpcModule::RegisterRequest* request,
                  RpcModule::RegisterResponse* reply) override{
        //code in here
        shared_ptr<WorkerInfo> workerInfo=make_shared<WorkerInfo>();
        workerInfo->ip=request->ip();
        workerInfo->port=request->port();
        workerInfo->status=INIT;
        CoorCtl->WorkerRegister(workerInfo);

        return Status::OK;
    }
    
    Status TaskDone(ServerContext* context, ServerReader<RpcModule::TaskResultRequest>* reader,
                  RpcModule::TaskRecvReply* reply) override{
        RpcModule::TaskResultRequest request;
        string ip;
        int port;
        int taskId;
        string task;
        TaskType type;
        vector<shared_ptr<WorkerInfo>>* wokerListFd=CoorCtl->GetWorkerList();
        vector<TaskInfo>* mapTasksFd=CoorCtl->GetMapTasks();
        vector<TaskInfo>* reduceTasksFd=CoorCtl->GetReduceTasks();
        auto reduce_it=reduceTasksFd->begin();
        int index=0;
        while (reader->Read(&request)) {
            //接受任务,需要判断是map还是reduce的任务结果
            type=static_cast<TaskType>(request.task_type());
            if(type==MAP){
                ip = request.ip();
                taskId=request.task_id();
                task=request.task();
                port = request.port();
                if(type==TaskType::MAP) {
                    auto map_it=mapTasksFd->begin()+taskId;
                    map_it->TaskStatus=2;
                    //reduce 任务设置
                    (*(reduce_it+index)).Tasks.push_back(task);
                    (*(reduce_it+index)).ip.push_back(ip);
                    (*(reduce_it+index)).port.push_back(port);
                    (*(reduce_it+index)).ReducetoMapIndex.push_back(taskId);
                    index++;
                }
                // cout<<ip<<":"<<port<<":"<<task<<endl;
            }  
            else{
                ip = request.ip();
                taskId=request.task_id();
                task=request.task();
                port = request.port();
                if(type==TaskType::REDUCE) {
                    auto reduce_it=reduceTasksFd->begin()+taskId;
                    reduce_it->TaskStatus=2;
                    {
                        CoorCtl->ReduceResultAdd();
                    }
                    CoorCtl->AddReduceResult(request.task());
                }
                // cout<<ip<<":"<<port<<":"<<task<<endl;
            }
        }
        //他完成了任务，又回到了初始化运行状态
        for(auto& worker: (*wokerListFd)){
            if(worker->ip==ip && worker->port==port){
                cout<<ip<<":"<<port<<"has been finished a task!"<<endl;
                worker->status=INIT;
                worker->tasksfd.Tasks.clear();
                worker->tasksfd.ip.clear();
                worker->tasksfd.port.clear();
                break;
            }
        }
        cout<<"worker has finished a task"<<endl;
        // 处理完成后设置回复
        reply->set_success(true);

        return Status::OK;
    }

private:
    Coordinator* CoorCtl;
};

class CoordinatorClient{
public:
    CoordinatorClient(std::shared_ptr<Channel> channel)
      : stub_(RpcModule::Worker::NewStub(channel)) {}
    //设置你的样例
    int CheckWorkerAlive(){
        RpcModule::CheckAliveRequest args;
        args.set_status(true);

        RpcModule::CheckAliveReply reply;
        ClientContext context;

        Status status=stub_->CheckAlive(&context,args, &reply);
        if (status.ok()) {
            return 0;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl;
            return -1;
        }
    }
  void AssignTask(const TaskInfo* fd) {
    ClientContext context;
    
    RpcModule::AssignTaskReply reply;

    // 创建一个流对象
    std::unique_ptr<ClientWriter<RpcModule::AssignTaskRequest>> writer(stub_->AssignTask(&context, &reply));

    // 发送请求
    for(int i=0;i<fd->Tasks.size();i++){
        RpcModule::AssignTaskRequest request;
        request.set_ip(fd->ip[i]);
        request.set_port(fd->port[i]);
        request.set_task_id(fd->TaskIndex);
        request.set_task(fd->Tasks[i]);
        request.set_task_type(fd->type);
        if (!writer->Write(request)) {
        break;
      }
    }
    // 通知服务器完成写入
    writer->WritesDone();

    // 获取响应
    Status status = writer->Finish();

    // 处理响应
    if (status.ok()) {
      std::cout << "Confirmation from server: " << std::endl;
    } else {
      std::cout << "Error from server: " << status.error_code() << ": " << status.error_message() << std::endl;
    }
  }
  void QuitClient(){
    RpcModule::QuitRequest args;
    args.set_quitsign(true);

    RpcModule::QuitReply reply;
    ClientContext context;

    Status status=stub_->QuitSign(&context,args, &reply);
  }

private:
    std::unique_ptr<RpcModule::Worker::Stub> stub_;
};



// grpc server register
void Coordinator::CoordinatorServer(){
    // Server logic here
    //server ip:port
    std::string server_address = absl::StrFormat("127.0.0.1:%d", 5555);
    ServerServiceImpl service(this);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();                           
}

//
// create a Coordinator.
// main/mrcoordinator.cpp calls this function.
// nReduce is the number of reduce tasks to use.
//
Coordinator::Coordinator(const std::vector<std::string> filenames,int nReduce):nReduce(nReduce){
    //初始化Map与Reduce任务
    int index=0;
    for(auto arg : filenames){
        vector<string> task={arg};
        TaskInfo MapTaskInfo(vector<string>{"127.0.0.1"},vector<int>{5555},MAP,task,index,0);//这个参数没有用，因为maptask任务直接从本机取就行
        MapTasks.push_back(MapTaskInfo);
        
        index++;
    }
    for(int i=0;i<nReduce;i++){
        TaskInfo ReduceTaskInfo(vector<string>{},vector<int>{},REDUCE,vector<string>{},0,0);
        ReduceTasks.push_back(ReduceTaskInfo);
    }
    //创建线程，定时检查worker工作状态
    thread t(&Coordinator::CheckClientAlive,this);
    //nReduce任务的数量
    CoordinatorServer();
}

Coordinator::~Coordinator(){

}

void Coordinator::TaskDistributed(){
    //任务分发
    // map分配的任务
    // 收集还没有完成的maptask,存入他的索引
    queue<int> undistributedMapTask;
    int runningMapTask=0;
    for(int i=0;i<MapTasks.size();i++){
        if(MapTasks[i].TaskStatus==0){
            undistributedMapTask.push(i);
        }
        else if(MapTasks[i].TaskStatus==1){
            ++runningMapTask;
        }
    }  
    if(undistributedMapTask.size()!=0){//分配map工作
        for(auto& it:WorkerList){
            if(it->status==INIT&&undistributedMapTask.size()!=0){
                // 设置分配任务
                vector<string> task;

                int tmp=undistributedMapTask.front();
                undistributedMapTask.pop();

                it->tasksfd.type=MapTasks[tmp].type;
                it->tasksfd.ip=MapTasks[tmp].ip;
                it->tasksfd.port=MapTasks[tmp].port;
                MapTasks[tmp].TaskStatus=1;
                task=MapTasks[tmp].Tasks;
                it->tasksfd.TaskIndex=tmp;
                it->tasksfd.Tasks=task;
                it->status=RUNNING;

                it->Done=false;
                CoordinatorClient client(
                    grpc::CreateChannel(
                        it->ip+":"+to_string(it->port), grpc::InsecureChannelCredentials()
                    )
                );
                client.AssignTask(&(it->tasksfd));
                cout<<"assgin a task to worker"<<it->ip+":"+to_string(it->port)<<endl;
            }
        }
    }
    else if(undistributedMapTask.size()==0&&runningMapTask==0){
        //检查没有分配的reduce任务
        //打印信息
        // for(int i=0;i<ReduceTasks.size();++i){
        //     for(int j=0;j<ReduceTasks[i].Tasks.size();++j){
        //         cout<<ReduceTasks[i].ip[j]<<":"<<ReduceTasks[i].port[j]<<endl;
        //         cout<<ReduceTasks[i].Tasks[j]<<endl;
        //     }
        //     cout<<endl;
        // }

        //检查所有的Maptask的文件服务器是不是还在
        // 工作
        if(ReduceTasks.size()>0){
            int index=0;
            bool map_all_done=true;
            while (index < ReduceTasks[0].ip.size()) {
                if (!CheckFileServer(ReduceTasks[0].ip[index], ReduceTasks[0].port[index])) {
                    std::cout << "map task id:" << ReduceTasks[0].ReducetoMapIndex[index]
                            << " file server " << ReduceTasks[0].ip[index] << ":" << ReduceTasks[0].port[index]
                            << " is down!" << std::endl;

                    // 重置 map 任务
                    int mapIndex = ReduceTasks[0].ReducetoMapIndex[index];
                    MapTasks[mapIndex].TaskStatus = 0;
                    std::cout << MapTasks[mapIndex].ip[0] << ":" << MapTasks[mapIndex].port[0]
                            << " " << MapTasks[mapIndex].Tasks[0] << std::endl;

                    // 清除 reduce 中过期的信息
                    for (size_t j = 0; j < ReduceTasks.size(); ++j) {
                        // std::cout << "begin erase: " << ReduceTasks[j].Tasks.size()
                        //     << " " << ReduceTasks[j].ip.size() << " " << ReduceTasks[j].port.size() << std::endl;
                        ReduceTasks[j].ip.erase(ReduceTasks[j].ip.begin() + index);
                        ReduceTasks[j].port.erase(ReduceTasks[j].port.begin() + index);
                        ReduceTasks[j].Tasks.erase(ReduceTasks[j].Tasks.begin() + index);
                        ReduceTasks[j].ReducetoMapIndex.erase(ReduceTasks[j].ReducetoMapIndex.begin() + index);
                        // std::cout << "After erase: " << ReduceTasks[j].Tasks.size()
                        //     << " " << ReduceTasks[j].ip.size() << " " << ReduceTasks[j].port.size() << std::endl;
                    }

                    std::cout << "After erase: " << ReduceTasks[0].Tasks.size()
                                << " " << ReduceTasks[0].ip.size() << " " << ReduceTasks[0].port.size() << std::endl;
                    map_all_done=false;
                    } 
                    else {
                        ++index;
                    }
            }
            if(map_all_done==false){
                return ;
            }
        }

        queue<int> undistributedReduceTask;
        int runningReduceTask=0;
        for(int i=0;i<ReduceTasks.size();i++){
            if(ReduceTasks[i].TaskStatus==0){
                undistributedReduceTask.push(i);
            }
            else if(ReduceTasks[i].TaskStatus==1){
                ++runningReduceTask;
            }
        }  
        for(auto& it:WorkerList){
            if(it->status==INIT&&undistributedReduceTask.size()!=0){
                // 设置分配任务
                vector<string> task;
                int tmp=undistributedReduceTask.front();
                undistributedReduceTask.pop();

                it->tasksfd.type=ReduceTasks[tmp].type;
                it->tasksfd.ip=ReduceTasks[tmp].ip;
                it->tasksfd.port=ReduceTasks[tmp].port;
                ReduceTasks[tmp].TaskStatus=1;
                task=ReduceTasks[tmp].Tasks;
                it->tasksfd.TaskIndex=tmp;
                it->tasksfd.Tasks=task;
                it->status=RUNNING;

                it->Done=false;
                CoordinatorClient client(
                    grpc::CreateChannel(
                        it->ip+":"+to_string(it->port), grpc::InsecureChannelCredentials()
                    )
                );
                client.AssignTask(&(it->tasksfd));
                cout<<"assgin a task to worker"<<it->ip+":"+to_string(it->port)<<endl;
            }
        }
    }
    {
        unique_lock<mutex>lock(mtx);
        if(reducedone==nReduce){
            //发送消息给worker，告诉他们结束结束运行了
            for(auto it = WorkerList.begin(); it != WorkerList.end();){
                CoordinatorClient client(
                    grpc::CreateChannel(
                        (*it)->ip+":"+to_string((*it)->port), grpc::InsecureChannelCredentials()
                    )
                );
                client.QuitClient();
                (*it).reset();
                it = WorkerList.erase(it); 
            }
            exit(0);
        }
    }
}


void Coordinator::CheckClientAlive(){
    //定时检查Worker是否在线，如果不在线，从WorkerList中删除该Worker
    while(1){
        cout<<WorkerList.size()<<endl;
        for(auto it = WorkerList.begin(); it != WorkerList.end();){
            {
                CoordinatorClient client(
                    grpc::CreateChannel(
                        (*it)->ip+":"+to_string((*it)->port), grpc::InsecureChannelCredentials()
                    )
                );
                int reply=client.CheckWorkerAlive();
                if(reply==-1){
                    cout<<(*it)->ip+":"+to_string((*it)->port)<<" offline"<<endl;
                    if((*it)->status == RUNNING) {//当你这个节点down了之后，需要把map任务重新置为0
                        if((*it)->tasksfd.type==MAP)MapTasks[(*it)->tasksfd.TaskIndex].TaskStatus = 0;
                        else ReduceTasks[(*it)->tasksfd.TaskIndex].TaskStatus =0;
                        
                    }
                    (*it).reset();
                    it = WorkerList.erase(it); 
                }else{
                    cout<<(*it)->ip<<" "<<(*it)->port<<":still alive"<<(*it)->status<<endl;
                    ++it;
                }
            }
            //这下面是分配任务的代码
        }
        for(int i=0;i<MapTasks.size();i++){
            cout<<"status:"<<MapTasks[i].TaskStatus<<endl;
        }

        {
                TaskDistributed();
        }
        sleep(1);
    }

}

bool Coordinator::Done(){
    return false;
}

bool Coordinator::CheckFileServer(string ip,int port){
        CoordinatorClient client(
            grpc::CreateChannel(
                ip+":"+to_string(port), grpc::InsecureChannelCredentials()
            )
        );
        int reply=client.CheckWorkerAlive();
        if(reply==-1){
            return false;
        }
        return true;
    }
