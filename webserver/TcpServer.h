#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Callback.h"
#include "TcpConnection.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <atomic>
using namespace std;

class TcpServer:noncopyable {
public:
    using ThreadInitCallback = function<void(EventLoop*)>;
    bool kReuseport;
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg, bool kreuseport = false);
    ~TcpServer();
    void setThreadInitCallback(const ThreadInitCallback& cb) {
        threadInitCallback = cb;
    }
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback = cb;
    }
    void setThreadNum(int numThreads);
    void start();
    EventLoop* getLoop() const {
        return Loop;
    }
    const string name() {
        return Name;
    }
    const string ipPort() {
        return IpPort;
    }
private:
    void newConnection(int sockfd, const InetAddress& clitAddr); 
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn); 
    using ConnectionMap = unordered_map<string, TcpConnectionPtr>;
    EventLoop* Loop; //baseloop
    const string IpPort;
    const string Name;
    unique_ptr<Acceptor> Acceptor_;
    shared_ptr<EventLoopThreadPool> Threadpool; //one loop per thread
    ThreadInitCallback threadInitCallback;
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    WriteCompleteCallback writeCompleteCallback;
    atomic_int Started;
    int NextConnId;
    ConnectionMap Connections;
};

#endif