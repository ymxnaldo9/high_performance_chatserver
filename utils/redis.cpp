#include "redis.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

Redis::Redis()
    :PublishContext(NULL), SubcribeContext(NULL) {}

Redis::~Redis() {
    if (PublishContext != NULL) {
        redisFree(PublishContext);
    }
    if (SubcribeContext != NULL) {
        redisFree(SubcribeContext);
    }
}

bool Redis::connect() {
    PublishContext = redisConnect("127.0.0.1", 6379);
    if (PublishContext == NULL) {
        cerr << "connect redis failed" << endl;
        return false;
    }
    SubcribeContext = redisConnect("127.0.0.1", 6379);
    if (SubcribeContext == NULL) {
        cerr << "connect redis failed" << endl;
        return false;
    }
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();
    cout << "connect redis successfully" << endl;
    return true;
}

bool Redis::publish(int channel, string message) {
    redisReply* reply = (redisReply*)redisCommand(PublishContext, "PUBLISH %d %s", channel, message.c_str());
    if (reply == NULL) {
        cerr << "publish command failed" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel) {
    if (redisAppendCommand(this->SubcribeContext, "SUBSCRIBE %d", channel) == REDIS_ERR) {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    int done = 0;
    while (!done) {
        if (redisBufferWrite(this->SubcribeContext, &done) == REDIS_ERR) {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel) {
    if (redisAppendCommand(this->SubcribeContext, "UNSUBSCRIBE %d", channel) == REDIS_ERR) {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    int done = 0;
    while (!done) {
        if (redisBufferWrite(this->SubcribeContext, &done) == REDIS_ERR) {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

void Redis::observer_channel_message() {
    redisReply *reply = NULL;
    while (redisGetReply(this->SubcribeContext, (void**)&reply) == REDIS_OK) {
        if (reply != NULL && reply->element[2] != NULL && reply->element[2]->str != NULL) {
            NotifyMessageHandler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
}

void Redis::init_notify_handler(function<void(int, string)> fn) {
    this->NotifyMessageHandler = fn;
}