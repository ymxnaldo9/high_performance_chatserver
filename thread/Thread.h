#ifndef THREAD_H
#define THREAD_H

#include "../webserver/noncopyable.h"
#include <functional>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
using namespace std;

class Thread:noncopyable {
public:
    using ThreadFunc = function<void()>;
    explicit Thread(ThreadFunc, const string& name = string());
    ~Thread();
    void start();
    void join();
    bool started() const {
        return Started;
    }
    pid_t tid() {
        return Tid;
    }
    const string& name() const {
        return Name;
    }
    static int numCreated() {
        return NumCreated;
    }
private:
    bool Started;
    bool Joined;
    shared_ptr<thread> thread_;
    pid_t Tid;
    ThreadFunc Func;
    string Name;
    static atomic_int NumCreated;
    void SetDefaultName();
};

#endif