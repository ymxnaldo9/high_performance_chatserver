#include "EventLoopThread.h"
using namespace std;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const string& name)
    :Loop(NULL), Exiting(false), Thread(std::bind(&EventLoopThread::threadFunc, this), name), Mutex(), Cond(), Callback(cb) {}  

EventLoopThread::~EventLoopThread() {
    Exiting = true;
    if (Loop != NULL) {
        Loop->quit();
        thread_.join();
    }
}
    
EventLoop* EventLoopThread::StartLoop() {
    thread_.start();
    EventLoop *loop = NULL;
    {
        unique_lock<mutex> lock(Mutex);
        while (Loop == NULL) {
            Cond.wait(lock);
        }
        loop = Loop;
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (Callback) {
        Callback(&loop);
    }
    {
        unique_lock<mutex> lock(Mutex);
        Loop = &loop;
        Cond.notify_one();
    }
    loop.loop();//EventLoop loop->Poller poll   subloop
    unique_lock<mutex> lock(Mutex);
    Loop = NULL;
}