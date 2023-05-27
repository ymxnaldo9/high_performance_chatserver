#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <map>
#include <unordered_map>
#include "Channel.h"
#include <sys/epoll.h>
class Channel;
using namespace std;

class Poller:noncopyable {
public:
    using ChannelList = vector<Channel*>;
    int epfd;
    vector<struct epoll_event> EventList;
    unordered_map<int, Channel*> ChannelMap;
    Poller();
    ~Poller();
    void poll(ChannelList& activeChannelList);
    void AddChannel(Channel* p);
    void RemoveChannel(Channel* p);
    void UpdateChannel(Channel* p);
private:
    static const int kInitEventSize = 16;
};

#endif