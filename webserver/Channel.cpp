#include "Channel.h"
#include <sys/epoll.h>
#include <iostream>
using namespace std;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int val)
    :Loop(loop), fd(val) {}
Channel::~Channel() {}

void Channel::HandleEvent() {
    if (Tied) {
        shared_ptr<void> guard = Tie.lock();
        if (guard) {
            if ((events & EPOLLHUP) && !(events & EPOLLIN)) {
                cout << "Closed abnormally" << endl;
                CloseCallback();
            }
            if (events & EPOLLERR) {
                cout << "Event error" << endl;
                ErrorCallback();
            }
            if (events & (EPOLLIN | EPOLLPRI)) {
                ReadCallback();
            }
            if (events & EPOLLOUT) {
                WriteCallback();
            }
        }
    }
    else {
        if ((events & EPOLLHUP) && !(events & EPOLLIN)) {
            cout << "Closed abnormally" << endl;
            CloseCallback();
        }
        if (events & EPOLLERR) {
            cout << "Event error" << endl;
            ErrorCallback();
        }
        if (events & (EPOLLIN | EPOLLPRI)) {
            ReadCallback();
        }
        if (events & EPOLLOUT) {
            WriteCallback();
        }
    }
}

void Channel::tie(const shared_ptr<void>& obj) {
    Tie = obj;
    Tied = true;
}

void Channel::remove() {
    Loop->RemoveChannel(this);
}

void Channel::update() {
    Loop->UpdateChannel(this);
}