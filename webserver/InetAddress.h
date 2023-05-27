#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <string.h>
#include <strings.h>
using namespace std;

class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr)
        :Addr(addr) {}
    string toIp() const;
    string toIpPort() const;
    uint16_t toPort() const;
    const struct sockaddr_in* getSockAddr() const {
        return &Addr;
    }
    void setSockAddr(const struct sockaddr_in& addr) {
        Addr = addr;
    }
private:
    struct sockaddr_in Addr;
};

#endif