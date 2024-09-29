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

#include "coordinator.grpc.pb.h"
#include "worker.grpc.pb.h"

#include "config.h"

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

struct KeyValue{
public:
    string Key;
    string Value;
    KeyValue(string key, string value) : Key(key), Value(value) { };
};

typedef vector<KeyValue> (*MapFunc)(string filename,string contents);
typedef string (*ReduceFunc)(string key,string value);

class Worker{
public:
    Worker(MapFunc mapF, ReduceFunc reduceF);
    ~Worker()=default;
    void CallExample();
    void ClientServer();
    void CallRegister();
    WorkerInfo* GetWorkInfo(){
        return &workerInfo;
    };
    void GetTaskFile();     //获取文件
    void FinishTask();     //任务结束
    void TaskExecute();
    void Execute();         //执行线程

    void NotifyTask(){
        unique_lock<mutex> lck(mtx);
        task_ready=true;
        cv.notify_one();
    }
    //hash function
    uint32_t fnv1aHash(const char* key, size_t length) {
        const uint32_t FNV_PRIME_32 = 0x01000193;
        const uint32_t FNV_OFFSET_BASIS_32 = 0x811c9dc5;
        uint32_t hash = FNV_OFFSET_BASIS_32;

        for (size_t i = 0; i < length; ++i) {
            hash ^= static_cast<uint32_t>(static_cast<unsigned char>(key[i]));
            hash *= FNV_PRIME_32;
        }
        return hash;
    }

    // Wrapper function that takes a C++ string and returns an int
    int ihash(const std::string& key) {
        return static_cast<int>(fnv1aHash(key.c_str(), key.size()) & 0x7fffffff);
    }
    // bool CheckAlive();
private:
    MapFunc mapF_p;
    ReduceFunc reduceF_p;
    WorkerInfo workerInfo;
    vector<KeyValue>intermediate;
    vector<KeyValue>reduceintermediate;

    FILE* file;
    int nReduce=8;

    //线程变量
    mutex mtx;
    condition_variable cv;
    bool task_ready=false;
};