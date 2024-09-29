#include <iostream>
#include <memory>
#include <unistd.h>
#include "worker.h"
#include <dlfcn.h>

using namespace std;

int main(int argc, char* argv[]){
    if(argc < 2){
        cout<<"Usage: mrworker xxx.so"<<endl;
    }
    MapFunc mapF;
    ReduceFunc reduceF;
    void* handle = dlopen(argv[1], RTLD_LAZY);
    if (!handle) {
        cerr << "Cannot open library: " << dlerror() << '\n';
        exit(-1);
    }
    mapF = (MapFunc)dlsym(handle, "mapF");
    if (!mapF) {
        cerr << "Cannot load symbol 'hello': " << dlerror() <<'\n';
        dlclose(handle);
        exit(-1);
    }
    reduceF = (ReduceFunc)dlsym(handle, "reduceF");
    if (!mapF) {
        cerr << "Cannot load symbol 'hello': " << dlerror() <<'\n';
        dlclose(handle);
        exit(-1);
    }

    std::shared_ptr<Worker>  workerPtr=make_shared<Worker>(mapF,reduceF);


    return 0;
}