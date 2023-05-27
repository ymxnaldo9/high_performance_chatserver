#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

class Redis {
public:
    Redis();
    ~Redis();
    bool connect();
    bool publish(int channel, string message);
    bool subscribe(int channel);
    bool unsubscribe(int channel);
    void observer_channel_message();
    void init_notify_handler(function<void(int, string)> fn);
private:
    redisContext* PublishContext;
    redisContext* SubcribeContext;
    function<void(int, string)> NotifyMessageHandler;
};

#endif