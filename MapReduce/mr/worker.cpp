
#include "worker.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ServerReader;
using grpc::ClientWriter;

using namespace std;

bool cmp(KeyValue a, KeyValue b){
    return a.Key<b.Key;
}

class GetFileWorkerClient{
public:
    GetFileWorkerClient(std::shared_ptr<Channel> channel)
      : stub_(RpcModule::Worker::NewStub(channel)) {}
    string CallFile(string path){
        RpcModule::GetFileContentRequest args;
        args.set_file_path(path);

        RpcModule::GetFileContentReply reply;
        ClientContext context;

        Status status=stub_->GetFileContentRpc(&context,args,&reply);
        if (status.ok()) {
            return reply.content();
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl;
            return "";
        }
    }
private:
    std::unique_ptr<RpcModule::Worker::Stub> stub_;
};

class WorkerClient{
public:
    WorkerClient(std::shared_ptr<Channel> channel)
      : stub_(RpcModule::Coordinator::NewStub(channel)) {}
    //设置你的样例
    int CallExample(){
        RpcModule::ExampleArgs args;
        args.set_x(99);

        RpcModule::ExampleReply reply;
        ClientContext context;

        Status status=stub_->Example(&context,args,&reply);
        if (status.ok()) {
            return reply.y();
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl;
            return -1;
        }
    }
    int CallRegister(WorkerInfo info){
        RpcModule::RegisterRequest args;
        args.set_ip(info.ip);
        args.set_port(info.port);

        RpcModule::RegisterResponse reply;
        ClientContext context;

        Status status=stub_->Register(&context,args,&reply);
        if (status.ok()) {
            return 0;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                        << std::endl;
            return -1;
        }
    }
    int TaskDone(const WorkerInfo* fd) {
        ClientContext context;
        RpcModule::TaskRecvReply reply;
        // 创建一个流对象
        std::unique_ptr<ClientWriter<RpcModule::TaskResultRequest>> writer(stub_->TaskDone(&context, &reply));

        // 发送请求
        for(int i=0;i<fd->tasksfd.TaskResults.size();i++){
            RpcModule::TaskResultRequest request;
            request.set_ip(fd->ip);
            request.set_port(fd->port);
            request.set_task_id(fd->tasksfd.TaskIndex);
            request.set_task(fd->tasksfd.TaskResults[i]);
            request.set_task_type(fd->tasksfd.type);
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
            return 1;
        } else {
            std::cout << "Error from server: " << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
  }
private:
    std::unique_ptr<RpcModule::Coordinator::Stub> stub_;
};

// rpc server defintion
class ClientServiceImpl final : public RpcModule::Worker::Service{
    // 对服务的改写
 public:   
    ClientServiceImpl(Worker* WorkerCtl){
        this->WorkerCtl=WorkerCtl;
    };
private:
    Status CheckAlive(ServerContext* context, const RpcModule::CheckAliveRequest* request,
                RpcModule::CheckAliveReply* reply) override{
        reply->set_status(true);
        return Status::OK;
    }
    Status AssignTask(ServerContext* context, ServerReader<RpcModule::AssignTaskRequest>* reader,
                  RpcModule::AssignTaskReply* reply) override{
        RpcModule::AssignTaskRequest request;
        WorkerInfo* WI=WorkerCtl->GetWorkInfo();
        cout<<"accept a task"<<endl;
        while (reader->Read(&request)) {
            //接受任务
            WI->tasksfd.ip.push_back(request.ip());
            WI->tasksfd.port.push_back(request.port());
            WI->tasksfd.TaskIndex=request.task_id();
            WI->tasksfd.Tasks.push_back(request.task());
            WI->tasksfd.type=static_cast<TaskType>(request.task_type());
            WI->tasksfd.TaskStatus=0;
            cout<<WI->tasksfd.ip[WI->tasksfd.ip.size()-1]<<":"<<WI->tasksfd.port[WI->tasksfd.port.size()-1]<< WI->tasksfd.Tasks[WI->tasksfd.Tasks.size()-1]<<" "<<WI->tasksfd.type<<endl;
        }

        // 处理完成后设置回复
        reply->set_success(true);
        //处理完之后，对实际函数工作线程发送启动信号
        WorkerCtl->NotifyTask();

        return Status::OK;
    }

    Status QuitSign(ServerContext* context, const RpcModule::QuitRequest* request,
                  RpcModule::QuitReply* reply) override{
        exit(0);
    }

    Status GetFileContentRpc(ServerContext* ServerContext,const RpcModule::GetFileContentRequest* request,
                            RpcModule::GetFileContentReply *reply) override{
        std::string path = request->file_path();
        
        // Allocate memory for the filename
        char* filename = new char[path.length() + 1];
        strcpy(filename, path.c_str());

        // Open the file
        FILE* file = fopen(filename, "r");
        if (file == nullptr) {
            std::cout << "Error: Could not open file " << filename << std::endl;
            delete[] filename;
            return Status(grpc::StatusCode::NOT_FOUND, "File not found");
        }

        // Determine the file size
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Allocate memory for the file contents
        char* contents = new char[fileSize + 1];
        fread(contents, fileSize, 1, file);
        contents[fileSize] = '\0'; // Null-terminate the string

        // Close the file
        fclose(file);

        // Set the content in the reply message
        reply->set_content(std::string(contents));
        reply->set_success(true);

        // Clean up allocated memory
        delete[] filename;
        delete[] contents;

        return Status::OK;
    }
    
private:
    Worker* WorkerCtl;
};

void Worker::ClientServer(){
        // Server logic here
    //server ip:port
    std::string server_address = workerInfo.ip+":"+to_string(workerInfo.port);
    ClientServiceImpl service(this);

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
// main/mrworker.cpp calls this function.
//
Worker::Worker(MapFunc mapF, ReduceFunc reduceF) : mapF_p(mapF), reduceF_p(reduceF) {
    // Your worker implementation here.
    {
        //设置当前的工作者的相关信息
        srand(static_cast<unsigned int>(time(nullptr)));
        workerInfo.ip = "127.0.0.1";
        int min_port=5556, max_port=30000;
        workerInfo.port = min_port+(rand()%(max_port-min_port+1));
    }

    CallExample();
    CallRegister();

    thread t(&Worker::Execute,this);
    ClientServer();
}

void Worker::CallExample(){
    WorkerClient client(
        grpc::CreateChannel(
            "127.0.0.1:5555", grpc::InsecureChannelCredentials()
        )
    );
    int reply=client.CallExample();
    cout<<"recive:"<<reply<<endl;
}

void Worker::CallRegister(){
    WorkerClient client(
        grpc::CreateChannel(
            "127.0.0.1:5555", grpc::InsecureChannelCredentials()
        )
    );
    int reply=client.CallRegister(this->workerInfo);
    cout<<"register:"<<reply<<endl;

}

void Worker::GetTaskFile(){
    //这里出现了一个没有cout就会报错的问题，经查询得知，是因为没有初始化fileaname
    // cout<<workerInfo.tasksfd.type<<" "<<workerInfo.tasksfd.Tasks[0]<<endl;
    // 内存分配问题：
    // filename 指针没有分配内存就直接使用 strcpy，这会导致段错误。
    // 调试信息的作用：
    // 在你的代码中，cout 输出实际上起到了一个“副作用”，即它会触发一些内部缓冲区的刷新或同步操作，从而暴露出潜在的问题。
    // 但是，这不是解决问题的根本方法。
    if(workerInfo.tasksfd.type==MAP){//启动Map函数
        // char* filename;
        char* filename = new char[workerInfo.tasksfd.Tasks[0].length() + 1];
        strcpy(filename, workerInfo.tasksfd.Tasks[0].c_str());
        file=fopen(filename,"r");
        if(file==NULL){
            cout<<"Error: Could not open file "<<filename<<endl;
            this->workerInfo.tasksfd.TaskStatus=-1;
        }
        delete[] filename;
        // cout<<filename<<endl;
    }else{//ReduceTask
        for(int i=0;i<workerInfo.tasksfd.Tasks.size();i++){
            GetFileWorkerClient client(
            grpc::CreateChannel(
                workerInfo.tasksfd.ip[i]+":"+to_string(workerInfo.tasksfd.port[i]), grpc::InsecureChannelCredentials()
                )
            );
            string content=client.CallFile(workerInfo.tasksfd.Tasks[i]);
            if(content=="")exit(0);//懒得做一个回复任务失败的消息，直接推出，coor也会知道这个工作者没有完成任务
            std::istringstream iss(content);
            std::string line;
            while (std::getline(iss, line)) {
                std::istringstream lineStream(line);
                string Key,Value;
                if (lineStream >> Key >> Value) {
                    KeyValue KV(Key,Value);
                    reduceintermediate.emplace_back(KV);
                } else {
                    std::cerr << "Error parsing line: " << line << std::endl;
                }
            }
            sort(reduceintermediate.begin(), reduceintermediate.end(),cmp);
            cout<< workerInfo.tasksfd.ip[i]+":"+to_string(workerInfo.tasksfd.port[i])<<" "<<workerInfo.tasksfd.Tasks[i]<<endl;
        }
    }
}
void Worker::TaskExecute(){//map需要这一步
    if(workerInfo.tasksfd.type==MAP){
        fseek(file,0,SEEK_END);
        long fileSize=ftell(file);
        fseek(file,0,SEEK_SET);
        char* contents=new char[fileSize+1];
        fread(contents,fileSize,1,file);
        contents[fileSize]='\0';
        fclose(file);
        intermediate=mapF_p("MapTask",contents);
        // cout<<intermediate.size()<<intermediate[0].Key<<intermediate[0].Value<<endl;

    }

    workerInfo.tasksfd.TaskStatus=1;
}

void Worker::FinishTask(){
    char filename[100];
    if(workerInfo.tasksfd.type==MAP){
        sort(intermediate.begin(),intermediate.end(),cmp);//进行排序
        //进行hash分配到不同文件中
        // cout<<intermediate.size()<<intermediate[0].Key<<intermediate[0].Value<<endl;
        vector<FILE*>files;
        for(int i=0;i<nReduce;i++){
            sprintf(filename,"../../MapTask_Taskid%d_output.%d",workerInfo.tasksfd.TaskIndex,i);
            workerInfo.tasksfd.TaskResults.push_back(string(filename));
            files.push_back(fopen(filename,"w"));
        }
        
        for(int i=0;i<intermediate.size();i++){
            int index=ihash(intermediate[i].Key)%nReduce;
            // cout<<index<<endl;
            fprintf(files[index],"%s %s\n",intermediate[i].Key.c_str(),intermediate[i].Value.c_str());
        }

        for(int i=0;i<nReduce;i++){
            fclose(files[i]);
        }
    }
    else{
        sprintf(filename,"../../mr-out-%d",workerInfo.tasksfd.TaskIndex);
        FILE* ofile=fopen(filename,"w");
        workerInfo.tasksfd.TaskResults.push_back(string(filename));
        for(int i=0;i<reduceintermediate.size();i++){
            string value=reduceintermediate[i].Value;
            int j=i+1;
            for(;j<reduceintermediate.size()&&reduceintermediate[j].Key == reduceintermediate[i].Key;j++){
                value+=reduceintermediate[j].Value;
            }
            string output=reduceF_p(reduceintermediate[i].Key,value);
            fprintf(ofile,"%s %s\n",reduceintermediate[i].Key.c_str(),output.c_str());
            i=j;
        }
        fclose(ofile);
    }
    //%50 可能无法运行该任务
    int min_port=0, max_port=1000;
    int tmp=min_port+(rand()%(max_port-min_port+1));
    if(tmp<=150)exit(0);
}

void Worker::Execute(){
    //执行任务
    while(1){
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock,[&]{return task_ready;});
        }
        //获取任务文件
        GetTaskFile();
        if(workerInfo.tasksfd.TaskStatus!=-1){
            TaskExecute();//任务执行
            FinishTask();//任务打乱，并保存文件
        }

        //结束任务，调用coor的TaskDone函数
        WorkerClient client(
            grpc::CreateChannel(
                "127.0.0.1:5555", grpc::InsecureChannelCredentials()
            )
        );
        int reply=client.TaskDone(&(this->workerInfo));
        {
            lock_guard<mutex>lock(mtx);
            task_ready=false;
        }

        //任务结束之后删除文件
        if(reply==1&&workerInfo.tasksfd.type==REDUCE){
            for(int i=0;i<workerInfo.tasksfd.Tasks.size();i++)
            if (std::remove(workerInfo.tasksfd.Tasks[i].c_str()) == 0) {
                std::cout << "File removed: " << workerInfo.tasksfd.Tasks[i] << std::endl;
            } else {
                std::cerr << "Failed to remove file: " << workerInfo.tasksfd.Tasks[i] << std::endl;
            }
        }
        //清空任务状态
        intermediate.clear();
        reduceintermediate.clear();
        workerInfo.tasksfd.Tasks.clear();
        workerInfo.tasksfd.TaskResults.clear();
        workerInfo.tasksfd.ip.clear();
        workerInfo.tasksfd.port.clear();
        //任务结束后等待启动信号
    }
}