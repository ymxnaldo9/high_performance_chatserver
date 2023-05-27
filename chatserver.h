#ifndef CHATSERVER_H
#define CHATSERVER_H

#include "webserver/TcpServer.h"
#include "webserver/EventLoop.h"
#include <string>
using namespace std;

class ChatServer {
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
    void start();
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);
    TcpServer Server;
    EventLoop* Loop;
};

#endif