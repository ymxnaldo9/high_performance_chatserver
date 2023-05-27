#include "OfflineMessageModel.h"
#include "../mysql/Connectionpool.h"
using namespace std;

void OfflineMsgModel::insert(int userid, string msg) {
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg);
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        conn->update(sql);
    }
}

void OfflineMsgModel::remove(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        conn->update(sql);
    }
}

vector<string> OfflineMsgModel::query(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where id = %d)", userid);
    vector<string> vec;
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        MYSQL_RES *res = conn->query(sql);
        if (res != NULL) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != NULL) {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}