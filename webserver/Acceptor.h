#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "noncopyable.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include <functional>
using namespace std;

class Acceptor:noncopyable {
public:
    using NewConnectionCallback = function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop* loop, const InetAddress& listenaddr, bool reuseport);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        NewConnectionCallback_ = cb;
    }
    bool listenning() {
        return Listenning;
    }
    void listen();
private:
    void handleRead();
    EventLoop* Loop; //baseloop, defined by user
    Socket AcceptSocket;
    Channel AcceptChannel;
    NewConnectionCallback NewConnectionCallback_;
    bool Listenning;
};

#endif