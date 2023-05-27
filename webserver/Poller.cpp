#include "Poller.h"
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

#define TIMEOUT 1000

Poller::Poller()
    :epfd(epoll_create(256)), EventList(kInitEventSize), ChannelMap() {
        if (epfd < 0) {
            perror("epoll_create error");
            exit(1);
        }
    }

Poller::~Poller() {
    close(epfd);
}

void Poller::poll(ChannelList& activeChannelList) {
    int timeout = TIMEOUT;
    int ret = epoll_wait(epfd, &(*EventList.begin()), (int)EventList.size(), timeout);
    if (ret == -1) {
        if (errno != EINTR) {
            perror("epoll wait failed");
        }
    }
    for (int i = 0; i < ret; i++) {
        int events = EventList[i].events;
        Channel* p = (Channel*)EventList[i].data.ptr;
        int fd = p->Getfd();
        if (ChannelMap.find(fd) != ChannelMap.end()) {
            p->Setevents(events);
            activeChannelList.push_back(p);
        }
        else {
            cout << "Not find channel" << endl;
        }
    }
    if (ret == EventList.size()) {
        EventList.resize(EventList.size() * 2);
    }
}

void Poller::AddChannel(Channel* p) {
    int fd = p->Getfd();
    struct epoll_event ev;
    ev.events = p->Getevents();
    ev.data.fd = fd;
    ev.data.ptr = p;
    ChannelMap[fd] = p;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll add error");
        exit(1);
    }
}

void Poller::RemoveChannel(Channel* p) {
    int fd = p->Getfd();
    ChannelMap.erase(fd);
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
        perror("epoll del error");
        exit(1);
    }
}

void Poller::UpdateChannel(Channel* p) {
    int fd = p->Getfd();
    struct epoll_event ev;
    ev.events = p->Getevents();
    ev.data.fd = fd;
    ev.data.ptr = p;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll mod error");
        exit(1);
    }
}