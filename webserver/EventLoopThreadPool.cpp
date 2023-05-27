#include "EventLoopThreadPool.h"
using namespace std;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg)
    :BaseLoop(baseLoop), Name(nameArg), Started(false), NumThreads(0), Next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    Started = true;
    for (int i = 0; i < NumThreads; i++) {
        char buf[Name.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", Name.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        Threads.push_back(unique_ptr<EventLoopThread>(t));
        Loops.push_back(t->StartLoop());
    }
    if (NumThreads == 0 && cb) {
        cb(BaseLoop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = BaseLoop;
    if (!Loops.empty()) {
        loop = Loops[Next];
        Next++;
        if (Next >= Loops.size()) {
            Next = 0;
        }
    }
    return loop;
}

vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    if (Loops.empty()) {
        return vector<EventLoop*>(1, BaseLoop);
    }
    else {
        return Loops;
    }
}