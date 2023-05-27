#include "Socket.h"
#include <errno.h>

Socket::~Socket() {
    close(Sockfd);
}

void Socket::bindAddress(const InetAddress& localaddr) {
    if (bind(Sockfd, (struct sockaddr*)localaddr.getSockAddr(), sizeof(struct sockaddr_in)) != 0) {
        perror("bind error");
    }
}

void Socket::listen() {
    if (::listen(Sockfd, 1024) != 0) {
        perror("listen error");
    }
}

void Socket::setNonblocking(int connfd) {
    int flag = fcntl(connfd, F_GETFL);
    flag = flag | O_NONBLOCK;
    fcntl(connfd, F_SETFL, flag);
}

int Socket::accept(InetAddress* clit_addr) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    int cfd = ::accept(Sockfd, (struct sockaddr*)&addr, &len);
    setNonblocking(cfd);
    if (cfd >= 0) {
        clit_addr->setSockAddr(addr);
    }
    else {
        perror("accept error");
    }
    return cfd;
}

void Socket::shutdownWrite() {
    if (shutdown(Sockfd, SHUT_WR) < 0) {
        perror("shutdownwrite error");
    }
}

void Socket::setReuseAddr() {
    int opt = 1;
    setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::setReusePort() {
    int opt = 1;
    setsockopt(Sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::setKeepAlive() {
    int opt = 1;
    setsockopt(Sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}