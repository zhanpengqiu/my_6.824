#include "raft.h"
//这里用于跑测试用例的，可以定义你要运行多少个raft服务器

using namespace std;



int main(){
    
    vector<thread> threads;
    //启动五个rafter服务器
    for(int i=0;i<5;i++){
        threads.push_back(thread([i]{
            //按理说，创建之后就会一直保持rafter的运行
            Raft* raft=new Raft(i);

            delete raft;
        }));
    }

    //等待结束5个rafter服务器的运行
    for(int i=0;i<threads.size();i++){
        threads[i].join();
    }

    return 0;
}