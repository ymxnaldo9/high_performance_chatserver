#include "TcpConnection.h"
#include <functional>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

static EventLoop* checkLoopNotNull(EventLoop* loop) {
    if (loop == NULL) {
        perror("Mainloop is null");
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, const string& nameArg, int sockfd, const InetAddress& localAddr, const InetAddress& clitAddr)
    :Loop(checkLoopNotNull(loop)), Name(nameArg), state(kConnecting), Reading(true), socket_(new Socket(sockfd)), channel_(new Channel(loop, sockfd)),
    LocalAddr(localAddr), ClitAddr(clitAddr), HighWaterMark(64 * 1024 * 1024) {
        channel_->setReadCallback(bind(&TcpConnection::handleRead, this));
        channel_->setWriteCallback(bind(&TcpConnection::handleWrite, this));
        channel_->setCloseCallback(bind(&TcpConnection::handleClose, this));
        channel_->setErrorCallback(bind(&TcpConnection::handleError, this));
        socket_->setKeepAlive();
    }

TcpConnection::~TcpConnection() {}

void TcpConnection::send(const string& buf) {
    if (state == kConnected) {
        if (Loop->IsInloopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        }
        else {
            Loop->RunInloop(bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* message, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fault = false;
    if (state == kDisconnecting) {
        perror("Disconnected!");
        return;
    }
    if (!channel_->isWriting() && OutputBuffer.readableBytes() == 0) {
        nwrote = write(channel_->Getfd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback) {
                Loop->QueueInloop(bind(writeCompleteCallback, shared_from_this()));
            }
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    fault = true;
                }
            }
        }
    }
    if (!fault && remaining > 0) {
        size_t oldlen = OutputBuffer.readableBytes();
        if (oldlen + remaining >= HighWaterMark && oldlen < HighWaterMark && highWaterMarkCallback) {
            Loop->QueueInloop(bind(highWaterMarkCallback, shared_from_this(), oldlen + remaining));
        }
        OutputBuffer.append((char*)message + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (state == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::shutdown() {
    if (state == kConnected) {
        setState(kDisconnecting);
        Loop->RunInloop(bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::handleRead() {
    int saveErrno = 0;
    ssize_t n = InputBuffer.readFd(channel_->Getfd(), &saveErrno);
    if (n > 0) {
        messageCallback(shared_from_this(), &InputBuffer);
    }
    else if (n == 0) {
        handleClose();
    }
    else {
        errno = saveErrno;
        perror("Tcp handle read failed");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        int saveErrno = 0;
        ssize_t n = OutputBuffer.writeFd(channel_->Getfd(), &saveErrno);
        if (n > 0) {
            OutputBuffer.retrieve(n);
            if (OutputBuffer.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback) {
                    Loop->QueueInloop(bind(writeCompleteCallback, shared_from_this()));
                }
                if (state == kDisconnected) {
                    shutdownInLoop();
                }
            }
        }
    }
    else {
        perror("Tcp handle write error");
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback(connPtr);
    closeCallback(connPtr);  //TcpServer::removeConnection
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (getsockopt(channel_->Getfd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    }
    else {
        err = optval;
    }
}