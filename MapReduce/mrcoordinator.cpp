#include <iostream>
#include <memory>
#include <unistd.h>
#include "coordinator.h"

using namespace std;

int main(int argc, char* argv[]){
    if(argc<2){
        cout<<"Usage: mrsequential inputfile..."<<endl;
    }
    vector<string> filenames(argv + 1, argv + argc);
    shared_ptr<Coordinator> coord=make_shared<Coordinator>(filenames,8);
    while(coord->Done()==false){
        sleep(1);
    }
    sleep(1);

    return 0;
}
