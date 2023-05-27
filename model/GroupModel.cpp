#include "GroupModel.h"
#include "../mysql/Connectionpool.h"
#include <stdlib.h>
using namespace std;

bool GroupModel::createGroup(Group& group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        if (conn->update(sql)) {
            group.setId(mysql_insert_id(conn->getConn()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userid, int groupid, string role) {
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        conn->update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a inner join groupuser b on a.id = b.groupid where b.userid = %d", userid);
    vector<Group> groupVec;
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        MYSQL_RES *res = conn->query(sql);
        if (res != NULL) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != NULL) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    for (Group& group:groupVec) {
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a inner join groupuser b on a.id = b.userid where b.groupid = %d", group.getId());
        MYSQL_RES *res = conn->query(sql);
        if (res != NULL) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != NULL) {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

vector<int> GroupModel::queryGroupUsers(int userid, int groupid) {
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupid, userid);
    vector<int> idVec;
    shared_ptr<Connection> conn = Connectionpool::getConnectionpool()->getConnection();
    if (conn) {
        MYSQL_RES *res = conn->query(sql);
        if (res != NULL) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != NULL) {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}