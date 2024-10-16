//
// this is an outline of the API that raft must expose to
// the service (or tester). see comments below for
// each of these functions for more details.
//
// rf = Make(...)
//   create a new Raft server.
// rf.Start(command interface{}) (index, term, isleader)
//   start agreement on a new log entry
// rf.GetState() (term, isLeader)
//   ask a Raft for its current term, and whether it thinks it is leader
// ApplyMsg
//   each time a new entry is committed to the log, each Raft peer
//   should send an ApplyMsg to the service (or tester)
//   in the same server.
//
#include "./include/raft.h"
#include "rpc.h"
#include "timer.h"

using namespace std;

bool cmp(const std::pair<int, int>& a, const std::pair<int, int>& b) {
    return a.first < b.first; // 将降序
}
bool removeFirstNLines(const std::string& filePath, size_t n) {
    // Step 1: Create a temporary file name
    std::string tempFilePath = filePath + ".tmp";

    // Step 2: Open the original file and the temporary file
    std::ifstream inFile(filePath);
    std::ofstream outFile(tempFilePath);

    if (!inFile.is_open() || !outFile.is_open()) {
        std::cerr << "Failed to open files." << std::endl;
        return false;
    }

    // Step 3: Skip the first N lines and copy the rest to the temporary file
    std::string line;
    size_t lineCount = 0;
    while (std::getline(inFile, line)) {
        if (++lineCount > n) {
            outFile << line << std::endl;
        }
    }

    inFile.close();
    outFile.close();

    // Step 4: Replace the original file with the temporary file
    if (std::rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
        std::cerr << "Failed to rename the temporary file." << std::endl;
        return false;
    }

    return true;
}
// the service or tester wants to create a Raft server. the ports
// of all the Raft servers (including this one) are in peers[]. this
// server's port is peers[me]. all the servers' peers[] arrays
// have the same order. persister is a place for this server to
// save its persistent state, and also initially holds the most
// recent saved state, if any. applyCh is a channel on which the
// tester or service expects Raft to send ApplyMsg messages.
// Make() must return quickly, so it should start goroutines
// for any long-running work.

Raft::Raft(int me):me(me){ 
    // Your initialization code here (3A, 3B, 3C).
    //所有参数初始化
    //一开始给网络分配ip与port，然后分配主机id，这样每个机器都有了一个唯一的标识

    //然后启动投票线程，发给自己知道的所有的主机，获取他们的投票，确定leader。

    //如果leader确定了之后，就立马向其他主机发送心跳包，宣告自己是leader，
    //此时其他的服务器知道以及产生一个leader了，就将自己设置为follower。
    //在此过程中，leader会不断的给follower发送心跳报，表明自己存活

    //然后就开始报文同步（这一部分的实现放在ticker中）

	// initialize from state persisted before a crash

	// start ticker goroutine to start elections
    Init();

    //运行计时器
    timer=new Timer(this);
}

Raft::~Raft(){
    delete timer;
}

void Raft::Run(){
    thread thread_ticker(&Raft::ticker,this);
    thread thread_client(&Raft::ClientServer,this);                         //启动服务器进程
    thread thread_randomaddlog(&Raft::RandomAddLog,this);
    thread thread_printlog(&Raft::PrintLog,this);
    thread thread_persistlog(&Raft::LogPersist ,this);
    thread thread_applylog(&Raft::ApplyLogLoop, this);
    thread thread_makesnapshot(&Raft::MakeSnapShot, this);

    // 等待两个线程结束
    thread_ticker.join();
    thread_client.join();
    thread_randomaddlog.join();
    thread_printlog.join();
    thread_persistlog.join();
    thread_applylog.join();
    thread_makesnapshot.join();
}

void Raft::Init(){
    //数据初始化
    string filepath="/home/qzp/lab/study/src/raft/server.txt";
    ReadServerConfig(filepath);
    raftertype=FOLLOWER;
    votesReceived=0;
    majorityVotes=RAFTSERVERNUM/2+1;
    currentTerm=0;
    votedFor=-1;
    logFilePath="/home/qzp/lab/study/src/raft/Node"+to_string(GetMyId())+"_log.txt";
    
    //按理来说这个log应该是从文件中读取的，后续补上
    // std::vector<LogEntry>log;
    commitIndex=-1;
    lastApplied=-1;
    for(int i=0;i<RAFTSERVERNUM;i++){
        nextIndex.push_back(0);
        matchIndex.push_back(-1);
    }

    //初始化状态机
    state_machine=make_shared<StateMachine>();
}

State Raft::GetState(){
    State state;
    state.term=currentTerm;
    state.isleader=raftertype;
    return state;
}

/// @brief rpc网络服务，所有的处理相关的服务都放在rpc.h头文件里面
void Raft::ClientServer(){
    // Server logic here
    std::string server_address = id_netmsg[me].ip+":"+to_string(id_netmsg[me].port);
    //这个ClientServiceImpl使用的是rpc.h中定义的
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

// example RequestVote RPC handler.
void Raft::SendRequestVote(){
	// Your code here (3A, 3B).
    //启用多个线程发送请求投票消息
    vector<thread> threads;
    //重置投票计数
    SetRafterTypeChangedFlag(false);
    this->ResetVotesReceived();
    this->AddVotesReceived();//投自己一票
    for(int i=0;i<id_netmsg.size();++i){
        //给你所有的相邻的节点发送请求
        if(i!=me){
            threads.push_back(thread([this,i]{
                RafterClient client(
                grpc::CreateChannel(
                    id_netmsg[i].ip+":"+to_string(id_netmsg[i].port), grpc::InsecureChannelCredentials()),
                    this    //传入当前raft句柄给处理请求的
                );
                // cout<<"Node "<<me<<"request vote from"<<i<<endl;
                int reply=client.CallVoted();
            }));
        }
    }
    //退出线程
    for(auto& t:threads){
        t.join();
    }
    //如果没有获得多数投票，则进入追随者状态
    cout<<"Node "<<me<<" vote "<<votesReceived<<" majorvote "<<majorityVotes<<" term "<<currentTerm<<endl;
    //保证只能从CANDIDATE状态进入LEADER状态
    if(this->GetVotesReceivedIsMajority()&&this->GetRafterType()==CANDIDATE){
        this->setRafterType(LEADER);
        this->SetLeaderId(me);
        // cout<<"Node "<<me<<" be a LEADER!"<<endl;
    }else{//否则进入了领导者状态
        this->setRafterType(FOLLOWER);
    }
}

void Raft::SendAppendEntries(){
    // Your code here (3A, 3B).
    vector<thread> threads;
    SetRafterTypeChangedFlag(false);
    for(int i=0;i<id_netmsg.size();i++){
        //给你所有的相邻的节点发送请求
        if(i!=me){
            threads.push_back(thread([this,i]{
                RafterClient client(
                grpc::CreateChannel(
                    id_netmsg[i].ip+":"+to_string(id_netmsg[i].port), grpc::InsecureChannelCredentials()),
                    this    //传入当前raft句柄给处理请求的
                );
                // cout<<"send a appendentries to "<<i<<endl;
                int reply=client.CallAppendEntries(i);
            }));
        }
    }
    for(auto& t:threads){
        t.join();
    }
    //每次appendentries的末尾，检查一下日志是否提交
    //如果大部分日志是提交的状态，那么就设置主机对应的新log设置为commited状态
    CheckCommitIndex();
}

// the service using Raft (e.g. a k/v server) wants to start
// agreement on the next command to be appended to Raft's log. if this
// server isn't the leader, returns false. otherwise start the
// agreement and return immediately. there is no guarantee that this
// command will ever be committed to the Raft log, since the leader
// may fail or lose an election. even if the Raft instance has been killed,
// this function should return gracefully.
//
// the first return value is the index that the command will appear at
// if it's ever committed. the second return value is the current
// term. the third return value is true if this server believes it is
// the leader.
StartMsg Raft::Start(){

}
// the tester doesn't halt goroutines created by Raft after each test,
// but it does call the Kill() method. your code can use killed() to
// check whether Kill() has been called. the use of atomic avoids the
// need for a lock.
//
// the issue is that long-running goroutines use memory and may chew
// up CPU time, perhaps causing later tests to fail and generating
// confusing debug output. any goroutine with a long-running loop
// should call killed() to check whether it should stop.
void Raft::Kill(){
    dead.store(1);//使用原子操作
    // Your code here, if desired.
}

bool Raft::Killed(){
    uint32_t z=dead.load();
    return z==1;
}

void Raft::ticker(){
    sleep(2);                                       //这里睡眠两秒是想要等所有的配置都启动生效
    static_cast<Timer*>(timer)->TickerLoop();       //强制转换
}
void Raft::ReadServerConfig(string path){
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int id;
        std::string ip;
        int port;

        if (!(iss >> id >> ip >> port)) {
            std::cerr << "Error parsing line: " << line << std::endl;
            continue;
        }
        NetMsg msg(ip, port);
        id_netmsg.insert(std::make_pair(id, msg));
    }
    file.close();
}
//这个checkcommitindex还不能简单的判断match来决定，还要判断提交处的index与term是不是相等
void Raft::CheckCommitIndex() {
    std::unordered_map<int, int> index_map;
    if(!IsLogReciveFlagAviailable())return;
    // 遍历 matchIndex，统计每个索引出现的次数
    for (const auto& index : matchIndex) {
        if (index_map.find(index) == index_map.end()) {
            index_map[index] = 1; // 初始化计数值为 1
        } else {
            ++index_map[index];
        }
    }

    // 确定最新的可提交索引
    int newCommitIndex = commitIndex;
    for (const auto& entry : index_map) {
        if (entry.second >= majorityVotes && GetIndexTerm(entry.first) == currentTerm) {
            newCommitIndex = std::max(newCommitIndex, entry.first);
        }
    }

    SetCommitIndex(newCommitIndex); // 更新提交索引
}

//这一部分还需要改进
//每个机器需要保证commit的日志是持久的
void Raft::LogPersist(){
    //日志的持久化
    //这里采用条件变量进行持久化，如果commitindex有改变，就持久到硬盘上
    shared_ptr<ofstream>file;
    while(1){
        //todo :
        unique_lock<mutex>lock(cv_commit_mtx);
        cv_commit.wait(lock);
        if(!IsLogReciveFlagAviailable())continue;

        //这里加上了这个！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！!
        if(last_commitindex<state_machine->GetIndex()-1)last_commitindex=state_machine->GetIndex()-1;

        cout<<GetTerm()<<" "<<GetMyId()<<" "<<commitIndex<<" "<<last_commitindex<<endl;
        //保存日志到硬盘上
        if(last_commitindex ==-1){//如果是新文件
            file=make_shared<ofstream>(logFilePath);
        }
        else{                     //如果不是新文件
            file=make_shared<ofstream>(logFilePath,ios::app);
        }
        // 检查文件是否成功打开
        if (!file->is_open()) {
                std::cerr << "无法打开文件 " << logFilePath << " 进行写入。\n";
                return;
        }
        cout<<"Node: "<<me<<" last_commitindex "<<last_commitindex<<" commitIndex: "<<commitIndex<<endl;
        vector<LogEntry> entries=GetEntries(last_commitindex+1,commitIndex+1);
        for(auto& entry : entries){
            (*file)<< entry.m_command<<endl;
        }
        last_commitindex=GetCommitIndex();
        file->close();
    }
}

//应用日志到状态机上,这一部分在后续可以继续该的，在ApplyLogCommand这里面改就行
void Raft::ApplyLogLoop(){
    //应用到状态机上！
    // todo：applying in status machine
    while(1){
        //每次获得commit事件的时候，触发一次apply，此时，将命令持久到状态机上
        //模拟睡眠，表示处理器性能不行导致的apply过慢的情况0.5秒apply一个log
        usleep(50000);
        if(!IsLogReciveFlagAviailable())continue;
        unique_lock<mutex> lock(apply_mtx);
        {
            unique_lock<mutex>lock(commit_mtx);
            if(commitIndex==lastApplied){
                continue;
            }
        }
        //如果apply落后于commitindex，则继续apply
        lastApplied++;
        vector<LogEntry> logentry=GetEntries(lastApplied,lastApplied+1);
        // cout<<"Node "<<GetMyId()<<" "<<"dasdfhasuifhsdkvfsiuafhosdvfsdafhga"<<endl;
        cout<<"Node : "<<me;
        state_machine->ApplyLogCommand(logentry[0],lastApplied);
        //将日志应用到状态机上，，每当完成一条命令之后，apply logindex就加一
        
    }
}

//开始制系统快照
//在制作快照的时候，raft服务器不会接受外部的log传入。在这里采用条件变量，控制raft何时继续接受客户端传入消息
//做快照的时候，要做成线程启动，定时检查log是不是超过了
//制作快照的时候，系统的log需要锁定，log不能继续增加了
void Raft::MakeSnapShot(){
    while(true){
        sleep(1);
        if(!IsLogReciveFlagAviailable())continue;
        if(state_machine->IsStartMakeSnapshot()){//如果日志超过一定数目，就删除状态机之前的日志，做到缩减日志的目的
            //1.删除内存中的log
            //在这里考虑安不安全的问题，顺序很重要
            LockLogReciveFlag();
            shared_ptr<ApplyMsg> applyMsg=make_shared<ApplyMsg>();
            shared_ptr<ApplyMsg> snapshotMsg=make_shared<ApplyMsg>();
            state_machine->GetApplyMsg(applyMsg);
            state_machine->GetSnapShotMsg(snapshotMsg);
            int apply_index=applyMsg->SnapshotIndex;
            int apply_term=applyMsg->SnapshotTerm;
            int snapshot_index=snapshotMsg->SnapshotIndex;
            int snapshot_term=snapshotMsg->SnapshotTerm;

            int delete_numof_log=apply_index-snapshot_index;
            {
                unique_lock<mutex> lock(log_mtx);
                cout<<"Node "<<GetMyId()<<" logsize "<<log.size()<<" delete_num_log "<<delete_numof_log<<endl;
                log.erase(log.begin(),log.begin()+delete_numof_log);
                cout<<"logsize "<<log.size()<<endl;
            }
            //3.更新状态机参数
            state_machine->UpdateSnapshot(applyMsg);
            state_machine->SaveSnapshot(me);
            //2.删除存盘上的log
            if(removeFirstNLines(logFilePath,delete_numof_log)){
                cout << "The first " << delete_numof_log << " lines have been removed successfully." << endl;
            }else{
                cout << "An error occurred during the removal process." << endl;
            }
            //更新了状态机快照的时候，此时raft里面的index以及不是原来的下标了，需要通过m_index找到
            //而matchindex是与目标主机通信时候需要甬道的的参数，所以这里需要更新以下
            for(int i=0;i<5;++i){
                if(i!=GetMyId()){
                    SetMatchIndex(i,GetPreLogIndex(i));
                }
            }
            // last_commitindex=state_machine->GetIndex()-1;
            UnlockLogReciveFlag();

            cout<<"success update logfile"<<endl;
        }
    }
}

void Raft::RandomAddLog(){
    // 每个一段时间增加日志
    //判断当前是不是leader,如果是领导者就可以添加。
    //后续如果有与客户端通信的机会的话，那么就可以由服务器告诉谁是leader，让主机面向对应的leader增加日志
    //每次间隔1-3秒发送一个增加log的消息
    while(1){
        int time=1+rand()%(2);
        // sleep(time);
        usleep(50000);
        if(GetRafterType()==LEADER){
            if(!IsLogReciveFlagAviailable())continue;//如果不可以接受，就循环
            //随机时间长度
            //随即发送多少日志
            LockLogReciveFlag();
            int num=1+rand()%3;
            int log_index;
            shared_ptr<ApplyMsg> statemachineLog=make_shared<ApplyMsg>();
            state_machine->GetApplyMsg(statemachineLog);
            {
                unique_lock<mutex>lock(log_mtx);
                if(log.size()==0){
                    if(statemachineLog!=nullptr&&statemachineLog->SnapshotIndex>0){
                        log_index=statemachineLog->SnapshotIndex;
                    }else{
                        log_index=-1;
                    }
                }else{
                    log_index=log[log.size()-1].m_index;
                }
            }
            int currentTerm=GetTerm();
            {
                unique_lock<mutex>lock(log_mtx);
                for(int i=0;i<num;i++){
                    log_index++;
                    LogEntry log_entry("Node "+to_string(GetMyId())+" Term: "+to_string(currentTerm)+" generated a cmd:"+to_string(log_index), currentTerm,log_index);
                    //不使用AddEntrites是因为在这个函数内部已经使用了mutex，所以在这里就不太适用
                    log.push_back(log_entry);
                    cout<<"Node "<<me<<" log: "<<"cmd"+to_string(log_index)<<endl;
                }  
            }
            UpdateMyMatchIndex();    
            UnlockLogReciveFlag();  
        }
    }
}
void Raft::PrintLog(){
    while(1){
        sleep(1);
        if(!IsLogReciveFlagAviailable())continue;
        unique_lock<mutex>lock(log_mtx);
        cout<<"Node "<<me<<" log size: "<<log.size()<<endl;
        for(auto& log:log){
            cout<<"Node "<<me<<" term: "<<log.m_term<<" Log: "<<log.m_command<<" "<<endl;
        } 
    }
}