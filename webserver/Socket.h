#ifndef SOCKET_H
#define SOCKET_H

#include "noncopyable.h"
#include "InetAddress.h"
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

class Socket:noncopyable {
public:
    explicit Socket(int sockfd)
        :Sockfd(sockfd) {}
    ~Socket();
    int fd() const {
        return Sockfd;
    }
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* clit_addr);
    void setNonblocking(int connfd);
    void shutdownWrite();
    void setReusePort();
    void setReuseAddr();
    void setKeepAlive();
private:
    const int Sockfd;
};

#endif 