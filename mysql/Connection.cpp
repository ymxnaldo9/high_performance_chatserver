#include "Connection.h"

Connection::Connection() {
    conn = mysql_init(NULL);
}

Connection::~Connection() {
    if (conn != NULL) {
        mysql_close(conn);
    }
}

bool Connection::connect(string ip, unsigned short port, string username, string password, string dbname) {
    MYSQL *p = mysql_real_connect(conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, NULL, 0);
    return p != NULL;
}

bool Connection::update(string sql) {
    if (mysql_query(conn, sql.c_str())) {
        LOG("update failed: " + sql);
        return false;
    }
    return true;
}

MYSQL_RES* Connection::query(string sql) {
    if (mysql_query(conn, sql.c_str())) {
        LOG("update failed: " + sql);
        return NULL;
    }
    return mysql_use_result(conn);
} 

void Connection::refreshAliveTime() { 
    alivetime = clock(); 
}

clock_t Connection::getAliveTime() { 
    return clock() - alivetime; 
}

MYSQL* Connection::getConn() {
    return conn;
}