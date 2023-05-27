#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include "noncopyable.h"
#include "EventLoop.h"
#include "../thread/Thread.h"
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>
class EventLoop;
using namespace std;

class EventLoopThread:noncopyable {
public:
    using ThreadInitCallback = function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const string& name = string());
    ~EventLoopThread();
    EventLoop* StartLoop();
private:
    void threadFunc();
    EventLoop* Loop;
    bool Exiting;
    Thread thread_;
    mutex Mutex;
    condition_variable Cond;
    ThreadInitCallback Callback;
};

#endif