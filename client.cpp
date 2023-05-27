#include "utils/json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>
#include "model/group.h"
#include "model/user.h"
#include "MsgType.h"
using namespace std;
using json = nlohmann::json;

User g_currentUser;
vector<User> g_currentUserFriendList;
vector<Group> g_currentUserGroupList;
bool isMainMenuRunning = false;
sem_t rwsem;
atomic_bool g_isLoginSuccess{false};

void readTaskHandler(int clientfd);
void mainMenu(int);
void showCurrentUserData();

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        cerr << "socket create error" << endl;
        exit(-1);
    }
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in))) {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }
    sem_init(&rwsem, 0, 0);
    thread readTask(readTaskHandler, clientfd); // pthread_create
    readTask.detach();                               // pthread_detach
    while (1) { 
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车
        switch (choice) {
            case 1: 
                {       // login业务
                    int id = 0;
                    char pwd[50] = {0};
                    cout << "userid:";
                    cin >> id;
                    cin.get(); // 读掉缓冲区残留的回车
                    cout << "userpassword:";
                    cin.getline(pwd, 50);
                    json js;
                    js["msgid"] = LOGIN_MSG;
                    js["id"] = id;
                    js["password"] = pwd;
                    string request = js.dump();
                    g_isLoginSuccess = false;
                    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                    if (len == -1) {
                        cerr << "send login msg error:" << request << endl;
                    }
                    sem_wait(&rwsem); // 等待信号量，由子线程处理完登录的响应消息后，通知这里
                    if (g_isLoginSuccess) {
                        isMainMenuRunning = true;
                        mainMenu(clientfd);
                    }
                }
                break;
            case 2: 
                {  // register业务
                    char name[50] = {0};
                    char pwd[50] = {0};
                    cout << "username:";
                    cin.getline(name, 50);
                    cout << "userpassword:";
                    cin.getline(pwd, 50);
                    json js;
                    js["msgid"] = REG_MSG;
                    js["name"] = name;
                    js["password"] = pwd;
                    string request = js.dump();
                    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                    if (len == -1) {
                        cerr << "send reg msg error:" << request << endl;
                    }  
                    sem_wait(&rwsem); // 等待信号量，子线程处理完注册消息会通知
                }
                break;
            case 3: // quit业务
                close(clientfd);
                sem_destroy(&rwsem);
                exit(0);
            default:
                cerr << "invalid input!" << endl;
                break;
        }
    }
    return 0;
}

void doRegResponse(json &responsejs) {
    if (responsejs["errno"].get<int>() != 0) {
        cerr << "name is already exist, register error!" << endl;
    }
    else {
        cout << "name register success, userid is " << responsejs["id"] << ", do not forget it!" << endl;
    }
}

void doLoginResponse(json &responsejs)
{
    if (responsejs["errno"].get<int>() != 0) {
        cerr << responsejs["errmsg"] << endl;
        g_isLoginSuccess = false;
    }
    else {  
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);
        if (responsejs.contains("friends")) {
            g_currentUserFriendList.clear();
            vector<string> vec = responsejs["friends"];
            for (string &str:vec) {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }
        if (responsejs.contains("groups")) {
            g_currentUserGroupList.clear();
            vector<string> vec1 = responsejs["groups"];
            for (string &groupstr:vec1) {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);
                vector<string> vec2 = grpjs["users"];
                for (string &userstr : vec2) {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }
        showCurrentUserData();
        if (responsejs.contains("offlinemsg")) {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str:vec) {
                json js = json::parse(str);
                if (ONE_CHAT_MSG == js["msgid"].get<int>()) {
                    cout << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                }
                else {
                    cout << "group message [" << js["groupid"] << "]:" << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoginSuccess = true;
    }
}

void readTaskHandler(int clientfd)
{
    while (1) {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);  // 阻塞了
        if (len == -1 || len == 0) {
            close(clientfd);
            exit(-1);
        }
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype) {
            cout << "[" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        if (GROUP_CHAT_MSG == msgtype) {
            cout << "group message [" << js["groupid"] << "]:" << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        if (LOGIN_MSG_ACK == msgtype) {
            doLoginResponse(js); 
            sem_post(&rwsem);    
            continue;
        }
        if (REG_MSG_ACK == msgtype) {
            doRegResponse(js);
            sem_post(&rwsem);    
            continue;
        }
    }
}

void showCurrentUserData() {
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!g_currentUserFriendList.empty()) {
        for (User &user : g_currentUserFriendList) {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!g_currentUserGroupList.empty()) {
        for (Group &group : g_currentUserGroupList) {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers()) {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

void help(int fd = 0, string str = "");// "help" command handler
void chat(int, string);// "chat" command handler
void addfriend(int, string);// "addfriend" command handler
void creategroup(int, string);// "creategroup" command handler
void addgroup(int, string);// "addgroup" command handler
void groupchat(int, string);// "groupchat" command handler
void loginout(int, string);// "loginout" command handler

unordered_map<string, string> commandMap = {
    {"help", "show all supporting commands"},
    {"chat", "one chat"},
    {"addfriend", "add friend"},
    {"creategroup", "create group"},
    {"addgroup", "add group"},
    {"groupchat", "chat in a group"},
    {"loginout", "login out"}};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

void mainMenu(int clientfd) {
    help();
    char buffer[1024] = {0};
    while (isMainMenuRunning) {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (idx == -1) {
            command = commandbuf;
        }
        else {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            cerr << "invalid input command!" << endl;
            continue;
        }
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

void help(int, string) {
    cout << "show command list >>> " << endl;
    for (auto &p:commandMap) {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void addfriend(int clientfd, string str) {
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}

void chat(int clientfd, string str) {
    int idx = str.find(":"); // friendid:message
    if (idx == -1) {
        cerr << "chat command invalid!" << endl;
        return;
    }
    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}

void creategroup(int clientfd, string str) {
    int idx = str.find(":");
    if (idx == -1) {
        cerr << "creategroup command invalid!" << endl;
        return;
    }
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1){
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}

void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}

void groupchat(int clientfd, string str) {
    int idx = str.find(":");
    if (idx == -1) {
        cerr << "groupchat command invalid!" << endl;
        return;
    }
    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}

void loginout(int clientfd, string) {
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else {
        isMainMenuRunning = false;
    }   
}
