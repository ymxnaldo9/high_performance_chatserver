#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include "noncopyable.h"
#include "EventLoop.h"
#include <memory>
class EventLoop;
using namespace std;

class Channel:noncopyable {
public:
    using EventCallback = function<void()>;
    Channel(EventLoop* loop, int val);
    ~Channel();
    void Setfd(int val) {
        fd = val;
    }
    int Getfd() {
        return fd;
    }
    void Setevents(int val) {
        events = val;
    }
    int Getevents() {
        return events;
    }
    void setReadCallback(EventCallback cb) {
        ReadCallback = move(cb);
    }
    void setWriteCallback(EventCallback cb) {
        WriteCallback = move(cb);
    }
    void setCloseCallback(EventCallback cb) {
        CloseCallback = move(cb);
    }
    void setErrorCallback(EventCallback cb) {
        ErrorCallback = move(cb);
    }
    void tie(const shared_ptr<void>&);
    void HandleEvent();
    void enableReading() {
        events |= kReadEvent;
        update();
    }
    void disableReading() {
        events &= ~kReadEvent;
        update();
    }
    void enableWriting() {
        events |= kWriteEvent;
        update();
    }
    void disableWriting() {
        events &= ~kWriteEvent;
        update();
    }
    void disableAll() {
        events = kNoneEvent;
        update();
    }
    bool isWriting() const {
        return events & kWriteEvent;
    }
    EventLoop* ownership() {
        return Loop;
    }
    void remove();
private:
    void update();
    int fd;
    int events;
    EventLoop* Loop;
    EventCallback ReadCallback;
    EventCallback WriteCallback;
    EventCallback ErrorCallback;
    EventCallback CloseCallback;
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    weak_ptr<void> Tie;
    bool Tied;
};

#endif