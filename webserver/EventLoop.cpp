#include "EventLoop.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
using namespace std;

__thread EventLoop* t_loopInThisThread = NULL;  //prevent more than one eventloop created by one thread

int CreateEventfd() {
    int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        perror("eventfd error");
        exit(1);
    }
    return evfd;
}

EventLoop::EventLoop()
    :Looping(false), Quit(false), CallingPendingFunctors(false), ThreadId(CurrentThread::tid()), poller(new Poller(this)), 
    WakeupFd(CreateEventfd()), WakeupChannel(new Channel(this, WakeupFd)) {
        if (!t_loopInThisThread) {
            t_loopInThisThread = this;
        }
        WakeupChannel->setReadCallback(std::bind(&EventLoop::HandleRead, this));
        WakeupChannel->enableReading();
    }

EventLoop::~EventLoop() {
    WakeupChannel->disableAll();
    WakeupChannel->remove();
    close(WakeupFd);
    t_loopInThisThread = NULL;
}

void EventLoop::loop() {
    Looping = true;
    Quit = false;
    while (!Quit) {
        ActiveChannelList.clear();
        poller->poll(ActiveChannelList);
        for (Channel* channel:ActiveChannelList) {
            channel->HandleEvent();
        }
        DoPendingFunctors();
    }
    Looping = false;
}

void EventLoop::quit() {
    Quit = true;
    if (!IsInloopThread()) {
        Wakeup();
    }
}

void EventLoop::RunInloop(Functor cb) {
    if (IsInloopThread()) {
        cb();
    }
    else {
        QueueInloop(cb);
    }
}

void EventLoop::QueueInloop(Functor cb) {
    {
        unique_lock<mutex> lock(Mutex);
        PendingFunctors.push_back(cb);
    }
    if (!IsInloopThread() || CallingPendingFunctors) {
        Wakeup();  //wake up thread of loop
    } 
}

void EventLoop::Wakeup() {
    uint64_t one = 1;
    ssize_t n = write(WakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        perror("wake up error");
        exit(1);
    }
}

void EventLoop::HandleRead() {
    uint64_t one = 1;
    ssize_t n = read(WakeupFd, &one, sizeof(one));
}

void EventLoop::DoPendingFunctors() {
    vector<Functor> functors;
    CallingPendingFunctors = true;
    {
        unique_lock<mutex> lock(Mutex);
        functors.swap(PendingFunctors);
    }
    for (const Functor& functor:functors) {
        functor();
    }
    CallingPendingFunctors = false;
}