#include "chatserver.h"
#include "chatservice.h"
#include <iostream>
#include <signal.h>
using namespace std;

void resetHandler(int) {
    ChatService::instance()->reset();
    exit(1);
}

int main() {
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(8000, "127.0.0.1");
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}