#ifndef CONNECTION_H
#define CONNECTION_H

#include <mysql/mysql.h>
#include <time.h>
#include <string>
#include <iostream>
using namespace std;

#define LOG(str)  cout << __FILE__ << ":" << __LINE__ << " " << __TIMESTAMP__ << ":" << str << endl;

class Connection {
public:
    Connection();
    ~Connection();
    bool connect(string ip, unsigned short port, string user, string password, string dbname);
    bool update(string sql);
    MYSQL_RES* query(string sql);
    void refreshAliveTime();
    clock_t getAliveTime();
    MYSQL* getConn(); 
private:
    MYSQL *conn;
    clock_t alivetime;
};

#endif