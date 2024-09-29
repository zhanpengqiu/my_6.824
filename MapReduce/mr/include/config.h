//my workerInfo
#include <iostream>
#include <vector>
using namespace std;
enum WorkerStatus{
    INIT=0,
    RUNNING,
    EXIT
};

enum TaskType{
    MAP=0,
    REDUCE
};

struct TaskInfo{
    vector<string> ip;
    vector<int> port;
    TaskType type;
    vector<string>Tasks;
    vector<string>TaskResults;
    vector<int> ReducetoMapIndex;
    int TaskIndex;              //记录worker完成的任务的序号
    int TaskStatus;             //任务状态，0未完成、1运行中、2完成，-1出错
    //构造函数，构造基本信息
    TaskInfo(){}
    TaskInfo(vector<string> ip,vector<int> port,TaskType type,vector<string>Tasks,int index,int status):ip(ip),port(port),type(type),Tasks(Tasks),TaskIndex(index),TaskStatus(status){}
};

struct WorkerInfo{
    // // Your definitions here.
    string ip;                  //记录worker的ip
    int port;                //记录worker的端口
    WorkerStatus status;        //记录worker状态

    TaskInfo tasksfd;

    bool Done;                  //记录worker是否完成
};