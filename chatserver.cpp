#include "chatserver.h"
#include "utils/json.hpp"
#include "chatservice.h"
#include <functional>
#include <string>
using namespace std;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
    :Server(loop, listenAddr, nameArg), Loop(loop) {
        Server.setConnectionCallback(bind(&ChatServer::onConnection, this, placeholders::_1));
        Server.setMessageCallback(bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2));
        Server.setThreadNum(4);
    }

void ChatServer::start() {
    Server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    string Buf = buf->retrieveAllAsString();
    json js = json::parse(Buf);
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js);
}
