#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include "noncopyable.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Callback.h"
#include "Buffer.h"
#include <memory>
#include <atomic>
#include <string>
#include <functional>
using namespace std;

class TcpConnection:noncopyable, public enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop, const string& nameArg, int sockfd, const InetAddress& localAddr, const InetAddress& clitAddr);
    ~TcpConnection();
    EventLoop* getLoop() const {
        return Loop;
    }
    const string name() const {
        return Name;
    }
    const InetAddress& localAddress() {
        return LocalAddr;
    }
    const InetAddress& clitAddress() {
        return ClitAddr;
    }
    bool connected() {
        return state == kConnected;
    }
    void send(const string& buf);
    void shutdown();
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback = cb;
    }
    void setCloseCallback(const CloseCallback& cb) {
        closeCallback = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t val) {
        highWaterMarkCallback = cb;
        HighWaterMark = val;
    }
    void connectEstablished();
    void connectDestroyed();
private:
    enum State {
        kDisconnected, 
        kConnecting,
        kConnected,
        kDisconnecting,
    };
    void setState(State state_) {
        state = state_;
    }
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    EventLoop* Loop; //subloop
    const string Name;
    atomic_int state;
    bool Reading;
    unique_ptr<Socket> socket_;
    unique_ptr<Channel> channel_;
    const InetAddress LocalAddr;
    const InetAddress ClitAddr;
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    WriteCompleteCallback writeCompleteCallback;
    CloseCallback closeCallback;
    HighWaterMarkCallback highWaterMarkCallback;
    size_t HighWaterMark;
    Buffer InputBuffer;
    Buffer OutputBuffer;
};

#endif