#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.h"
#include <vector>
using namespace std;

class  FriendModel {
public:
    void insert(int userid, int friendid);
    vector<User> query(int userid); //return friend list
};

#endif