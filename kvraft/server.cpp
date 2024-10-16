#include "server.hpp"

using namespace std;

KVServer::KVServer(int id){
    raft=make_shared<Raft>(id);
}