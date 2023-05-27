#include "Acceptor.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
using namespace std;

static int createNonblocking() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        perror("listen socket create err");
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenaddr, bool reuseport)
    :Loop(loop), AcceptSocket(createNonblocking()), AcceptChannel(loop, AcceptSocket.fd()), Listenning(false) {
        AcceptSocket.setReusePort();
        AcceptSocket.setReuseAddr();
        AcceptSocket.bindAddress(listenaddr);
        AcceptChannel.setReadCallback(bind(&Acceptor::handleRead, this));
    }

Acceptor::~Acceptor() {
    AcceptChannel.disableAll();
    AcceptChannel.remove();
}

void Acceptor::listen() {
    Listenning = true;
    AcceptSocket.listen();
    AcceptChannel.enableReading();
}

void Acceptor::handleRead() {
    InetAddress ClitAddr;
    int cfd = AcceptSocket.accept(&ClitAddr);
    if (cfd >= 0) {
        if (NewConnectionCallback_) {
            NewConnectionCallback_(cfd, ClitAddr);
        }
        else {
            close(cfd);
        }
    }
    else {
        perror("accept error");
    }
}