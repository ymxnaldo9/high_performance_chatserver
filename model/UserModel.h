#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.h"

class UserModel {
public:
    bool insert(User& user);
    User query(int id);
    bool updateState(User user);
    void resetState();
};

#endif