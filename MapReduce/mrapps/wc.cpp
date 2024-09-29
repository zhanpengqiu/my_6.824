#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <bits/stdc++.h>
#include <pthread.h>
#include <dlfcn.h>
using namespace std;
class KeyValue{
public:
    string Key;
    string Value;
    KeyValue(string key, string value) : Key(key), Value(value) { };
};
vector<string> split(const char* content,int length){
    string tmpStr="";
    vector<string> result;
    for(int i=0;i<length;i++){
        if((content[i]>='a'&&content[i]<='z')||(content[i]>='A'&&content[i]<='Z')){
            tmpStr+=content[i];
        }
        else{
            if(tmpStr!=""){
                result.push_back(tmpStr);
                tmpStr="";
            }
        }
    }
    return result;
};


extern "C" vector<KeyValue> mapF(string filename,string contents){
    vector<KeyValue> result;
    int start=0;
    vector<string> tmp;
    tmp=split(contents.c_str(),contents.size());
    for(int i=0;i<tmp.size();i++){
        KeyValue* keyValue=new KeyValue(tmp[i],"1");
        result.push_back(*keyValue);
        delete keyValue;
    }
    return result;
}

//返回的是key对应的大小
extern "C" string reduceF(string key,string value){
    return to_string(value.size());
}