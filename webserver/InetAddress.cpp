#include "InetAddress.h"
using namespace std;

InetAddress::InetAddress(uint16_t port, string ip) {
    bzero(&Addr, sizeof(Addr));
    Addr.sin_family = AF_INET;
    Addr.sin_port = htons(port);
    Addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

string InetAddress::toIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &Addr.sin_addr, buf, sizeof(buf));
    return buf;
}

string InetAddress::toIpPort() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &Addr.sin_addr, buf, sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ntohs(Addr.sin_port);
    sprintf(buf + end, ":u", port);
    return buf;
}

uint16_t InetAddress::toPort() const {
    return ntohs(Addr.sin_port);
}