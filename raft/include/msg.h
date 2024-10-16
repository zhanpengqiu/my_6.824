#pragma once
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
// as each Raft peer becomes aware that successive log entries are
// committed, the peer should send an ApplyMsg to the service (or
// tester) on the same server, via the applyCh passed to Make(). set
// CommandValid to true to indicate that the ApplyMsg contains a newly
// committed log entry.
//
// in part 3D you'll want to send other kinds of messages (e.g.,
// snapshots) on the applyCh, but set CommandValid to false for these
// other uses.
struct ApplyMsg{
    bool CommandValid;
    void* Command;
    int CommandIndex;

    // For 3D:
    bool SnapshotValid;
    string Snapshot;
    int SnapshotTerm;
    int SnapshotIndex;
};

class LogEntry{
public:
    LogEntry(string cmd = "", int term = -1,int index = -1):m_command(cmd),m_term(term),m_index(index){}
    string m_command;
    int m_term;
    int m_index;
};

enum RafterType{
    LEADER=0,
    FOLLOWER,
    CANDIDATE
};


//GetState 使用到的结构体
struct State{
    int term;
    RafterType isleader;
};


struct NetMsg{
    string ip;      // this end-point's ip
    int port;    // this end-point's port
    bool done;      // closed when Network is cleaned up
    NetMsg(string ip, int port,bool done=true): ip(ip), port(port){};
    NetMsg() : ip(""), port(0) {}
};

struct StartMsg{
    int index;
    int term;
    RafterType isLeader;
};

// example RequestVote RPC arguments structure.
// field names must start with capital letters!
struct RequestVoteRequest{
    // Your data here (3A, 3B).
    int term;
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;
};

// example RequestVote RPC reply structure.
// field names must start with capital letters!
struct RequestVoteReply{
    // Your data here (3A).
    int term;
    bool voteGranted;
};

//AppendEntries RPC request structure
struct AppendEntriesRequest{
    int term;
    int leaderId;
    int prevLogIndex;

    int prevLogTerm;
    vector<LogEntry> entries;

    int leaderCommit;
};

// AppendEntries RPC reply structure
struct  AppendEntriesReply{
    /* data */
    int term;
    bool success;
    int lastlogindex;
};
