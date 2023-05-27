#include "Connectionpool.h"
#include <stdio.h>
using namespace std;

Connectionpool* Connectionpool::getConnectionpool() {
    static Connectionpool pool;
    return &pool;
}

bool Connectionpool::loadConfigFile() {
    FILE *fp = fopen("mysql.cnf", "r");
    if (fp == NULL) {
        LOG("mysql.ini file is not exist!");
        return false;
    }
    while (!feof(fp)) {
        char line[1024] = {0};
        fgets(line, 1024, fp);
        string str = line;
        int idx = str.find('=', 0);
        if (idx == -1) {
            continue;
        }
        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx + 1, endidx - idx - 1);
        if (key == "ip") {
            ip = value;
        }
        else if (key == "port") {
            port = atoi(value.c_str());
        }
        else if (key == "username") {
            username = value;
        }
        else if (key == "password") {
            password = value;
        }
        else if (key == "dbname") {
            dbname = value;
        }
        else if (key == "initSize") {
            initSize = atoi(value.c_str());
        }
        else if (key == "maxSize") {
            maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime") {
            maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeout") {
            connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

Connectionpool::Connectionpool() {
    if (!loadConfigFile()) {
        return;
    }
    for (int i = 0; i < initSize; i++) {
        Connection *p = new Connection();
        p->connect(ip, port, username, password, dbname);
        p->refreshAliveTime();
        connectionQueue.push(p);
        connectionCnt++;
    }
    thread produce(bind(&Connectionpool::produceConnectionTask, this));
    produce.detach();
    thread scanner(bind(&Connectionpool::scannerConnectionTask, this));
    scanner.detach();
}

void* Connectionpool::produceConnectionTask() {
    while (1) {
        unique_lock<mutex> lock(queueLocker);
        while (!connectionQueue.empty()) {
            cv.wait(lock);
        }
        if (connectionCnt < maxSize) {
            Connection* p = new Connection();
            p->connect(ip, port, username, password, dbname);
            p->refreshAliveTime();
            connectionQueue.push(p);
            connectionCnt++;
        }
        cv.notify_all();
    }
    return NULL;
}

shared_ptr<Connection> Connectionpool::getConnection() {
    unique_lock<mutex> lock(queueLocker);
    while (connectionQueue.empty()) {
        if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(connectionTimeout))) {
            if (connectionQueue.empty()) {
                LOG("timeout");
                return NULL;
            }
        }
    }
    shared_ptr<Connection> sp(connectionQueue.front(), [&](Connection* pcon){
        unique_lock<mutex> lock(queueLocker);
        pcon->refreshAliveTime();
        connectionQueue.push(pcon);
    });
    connectionQueue.pop();
    cv.notify_all();
    return sp;
}

void* Connectionpool::scannerConnectionTask() {
    while (1) {
        this_thread::sleep_for(chrono::seconds(maxIdleTime));
        unique_lock<mutex> lock(queueLocker);
        while (connectionCnt > initSize) {
            Connection *p = connectionQueue.front();
            if (p->getAliveTime() >= (maxIdleTime * 1000)) {
                connectionQueue.pop();
                connectionCnt--;
                delete p;
            }
            else {
                break;
            }
        }
    }
}