#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>
using namespace std;

atomic_int Thread::NumCreated(0);

Thread::Thread(ThreadFunc func, const string& name)
    :Started(false), Joined(false), Tid(0), Func(move(func)), Name(name) {
        SetDefaultName();
    }

Thread::~Thread() {
    if (Started && !Joined) {
        thread_->detach();
    }
}

void Thread::start() {
    Started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = shared_ptr<thread>(new thread([&](){
        Tid = CurrentThread::tid();
        sem_post(&sem);
        Func();
    }));
    sem_wait(&sem);
}

void Thread::join() {
    Joined = true;
    thread_->join();
}

void Thread::SetDefaultName() {
    int num = ++NumCreated;
    if (Name.empty()) {
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        Name = buf;
    }
}