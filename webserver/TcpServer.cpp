#include "TcpServer.h"
#include <errno.h>
#include <strings.h>
using namespace std;

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if (loop == NULL) {
        perror("Mainloop is null");
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg, bool kreuseport)
    :Loop(loop), IpPort(listenAddr.toIpPort()), Name(nameArg), Acceptor_(new Acceptor(loop, listenAddr, kreuseport)), 
    Threadpool(new EventLoopThreadPool(Loop, Name)), connectionCallback(), messageCallback(), writeCompleteCallback(),
    threadInitCallback(), Started(0), NextConnId(1) {
        Acceptor_->setNewConnectionCallback(bind(&TcpServer::newConnection, this, placeholders::_1, placeholders::_2));
    }

TcpServer::~TcpServer() {
    for (auto item:Connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->RunInloop(bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    Threadpool->SetThreadNum(numThreads);
}

void TcpServer::start() {
    if (Started++ == 0) {
        Threadpool->start(threadInitCallback);
        Loop->RunInloop(bind(&Acceptor::listen, Acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& clitAddr) {
    EventLoop* ioLoop = Threadpool->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "-%s#%d", IpPort.c_str(), NextConnId);
    NextConnId++;
    string connName = Name + buf;
    sockaddr_in local;
    bzero(&local, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) {
        perror("getsockname failed");
    }
    InetAddress localAddr(local);
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, clitAddr));
    Connections[connName] = conn;
    conn->setConnectionCallback(connectionCallback);
    conn->setMessageCallback(messageCallback);
    conn->setWriteCompleteCallback(writeCompleteCallback);
    conn->setCloseCallback(bind(&TcpServer::removeConnection, this, placeholders::_1));
    ioLoop->RunInloop(bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    Loop->RunInloop(bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    Connections.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->QueueInloop(bind(&TcpConnection::connectDestroyed, conn));
}