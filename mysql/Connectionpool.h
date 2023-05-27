#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <string>
#include <queue>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <functional>
#include <stdlib.h>
#include "Connection.h"
using namespace std;

class Connectionpool {
public:
    static Connectionpool* getConnectionpool();
    shared_ptr<Connection> getConnection();
private:
    Connectionpool();
    bool loadConfigFile();
    string ip;
    unsigned short port;
    string username;
    string password;
    string dbname;
    int initSize;
    int maxSize;
    int maxIdleTime;
    int connectionTimeout;
    queue<Connection*> connectionQueue;
    mutex queueLocker;
    atomic_int connectionCnt; //number of connections
    condition_variable cv;
    void* produceConnectionTask();
    void* scannerConnectionTask();
};

#endif