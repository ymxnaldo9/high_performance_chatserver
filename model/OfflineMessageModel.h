#ifndef OFFLINE_MESSAGE_MODEL_H
#define OFFLINE_MESSAGE_MODEL_H

#include <string>
#include <vector>
using namespace std;

class OfflineMsgModel {
public:
    void insert(int userid, string msg);
    void remove(int userid);
    vector<string> query(int userid);
};

#endif