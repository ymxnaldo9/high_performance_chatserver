#include "UserModel.h"
#include "../mysql/Connectionpool.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

bool UserModel::insert(User& user) {
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", user.getName(), user.getPwd().c_str(), user.getState().c_str());
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        if (conn->update(sql)) {
            user.setId(mysql_insert_id(conn->getConn()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id) {
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        MYSQL_RES *res = conn->query(sql);
        if (res != NULL) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != NULL) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updateState(User user) {
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        if (conn->update(sql)) {
            return true;
        }
    }
    return false;
}

void UserModel::resetState() {
    char sql[1024] = "update user set state = 'offline' where id = 'online'";
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        conn->update(sql);
    }
}