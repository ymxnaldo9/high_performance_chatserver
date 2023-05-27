#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "noncopyable.h"
#include "Channel.h"
#include "Poller.h"
#include "../thread/CurrentThread.h"
#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>
class Channel;
using namespace std;

class EventLoop:noncopyable {
public:
    using Functor = function<void()>;
    EventLoop();
    ~EventLoop();
    void loop();  //begin eventloop
    void quit();  //quit eventloop
    void RunInloop(Functor cb);  //run cb
    void QueueInloop(Functor cb);  //put cb into queue
    void Wakeup();  //wakeup thread 
    void AddChannel(Channel *p) {
        poller->AddChannel(p);
    }
    void UpdateChannel(Channel *p) {
        poller->UpdateChannel(p);
    }
    void RemoveChannel(Channel *p) {
        poller->RemoveChannel(p);
    }
    bool IsInloopThread() const {
        return ThreadId == CurrentThread::tid();
    }
private:
    using ChannelList = vector<Channel*>;
    atomic_bool Looping;
    atomic_bool Quit;
    atomic_bool CallingPendingFunctors;
    const pid_t ThreadId;
    unique_ptr<Poller> poller;
    int WakeupFd; 
    unique_ptr<Channel> WakeupChannel;
    ChannelList ActiveChannelList;
    vector<Functor> PendingFunctors;  //All the functors in the loop
    mutex Mutex;  //lock, preserving the above vectors.
    void HandleRead();
    void DoPendingFunctors();
};

#endif