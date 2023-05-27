#ifndef EVENT_LOOP_THREAD_POOL_H
#define EVENT_LOOP_THREAD_POOL_H

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <functional>
#include <string>
#include <memory>
#include <vector>
using namespace std;

class EventLoopThreadPool:noncopyable {
public:
    using ThreadInitCallback = function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
    ~EventLoopThreadPool();
    void SetThreadNum(int numThreads) {
        NumThreads = numThreads;
    }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    EventLoop* getNextLoop();
    vector<EventLoop*> getAllLoops();
    bool started() const {
        return Started;
    }
    const string name() const {
        return Name;
    }
private:
    EventLoop* BaseLoop;
    string Name;
    bool Started;
    int NumThreads;
    int Next;
    vector<unique_ptr<EventLoopThread>> Threads;
    vector<EventLoop*> Loops;
};

#endif