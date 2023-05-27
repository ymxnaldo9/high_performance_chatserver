#include "FriendModel.h"
#include "../mysql/Connectionpool.h"
#include <stdlib.h>
using namespace std;

void FriendModel::insert(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        conn->update(sql);
    }
}

vector<User> FriendModel::query(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d", userid);
    vector<User> vec;
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        MYSQL_RES *res = conn->query(sql);
        if (res != NULL) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != NULL) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}