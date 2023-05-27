#include "chatservice.h"
#include "MsgType.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

ChatService* ChatService::instance() {  
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    MsgHandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({LOGINOUT_MSG, bind(&ChatService::loginout, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({ADD_FRIEND_MSG, bind(&ChatService::addFriend, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, placeholders::_1, placeholders::_2)});
    MsgHandlerMap.insert({GROUP_CHAT_MSG, bind(&ChatService::groupChat, this, placeholders::_1, placeholders::_2)});
    if (redis.connect()) {
        redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage, this, placeholders::_1, placeholders::_2));
    }
}

MsgHandler ChatService::getHandler(int msgid) {
    auto it = MsgHandlerMap.find(msgid);
    if (it == MsgHandlerMap.end()) {
        return [=](const TcpConnectionPtr& conn, json& js) {
            cout << "can not find handler";
        };
    }
    return MsgHandlerMap[msgid];
}

void ChatService::login(const TcpConnectionPtr& conn, json& js) {
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd) {
        if (user.getState() == "online") {
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "user already login in";
            conn->send(response.dump());
        }
        else {
            {
                lock_guard<mutex> lock(ConnMutex);
                UserConnMap.insert({id, conn});
            }
            redis.subscribe(id);
            user.setState("online");
            userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["id"] = user.getId();
            response["errno"] = 0;
            response["name"] = user.getName();
            vector<string> vec = offlineMsgModel.query(id);
            if (!vec.empty()) {
                response["offlinemsg"] = vec;
                offlineMsgModel.remove(id);
            }
            vector<User> userVec = friendModel.query(id);
            if (!userVec.empty()) {
                vector<string> friends;
                for (User& user:userVec) {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    friends.push_back(js.dump());
                }
                response["friends"] = friends;
            }
            conn->send(response.dump());
            vector<Group> groupuserVec = groupModel.queryGroups(id);
            if (!groupuserVec.empty()) {
                vector<string> groupV;
                for (Group& group:groupuserVec) {
                    json groupjs;
                    groupjs["id"] = group.getId();
                    groupjs["groupname"] = group.getName();
                    groupjs["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser& user:group.getUsers()) {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    groupjs["users"] = userV;
                    groupV.push_back(groupjs.dump());
                }
                response["groups"] = groupV;
            }
        }
    }
    else {
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password invalid";
        conn->send(response.dump());
    }
}

void ChatService::loginout(const TcpConnectionPtr& conn, json& js) {
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(ConnMutex);
        auto it = UserConnMap.find(userid);
        if (it != UserConnMap.end()) {
            UserConnMap.erase(it);
        }
    }
    redis.unsubscribe(userid);
    User user(userid, "", "", "offline");
    userModel.updateState(user);
}

void ChatService::reg(const TcpConnectionPtr& conn, json& js) {
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = userModel.insert(user);
    if (state) {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getId();
        response["errno"] = 0;
        conn->send(response.dump());
    }
    else {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn) {
    User user;
    {
        lock_guard<mutex> lock(ConnMutex);
        for (auto it = UserConnMap.begin(); it != UserConnMap.end(); it++) {
            if (it->second == conn) {
                user.setId(it->first);
                UserConnMap.erase(it);
                break;
            }
        }
    }
    redis.unsubscribe(user.getId());
    if (user.getId() != -1) {
        user.setState("offline");
        userModel.updateState(user);
    }
}

void ChatService::reset() {
    userModel.resetState();
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js) {
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(ConnMutex);
        auto it = UserConnMap.find(toid);
        if (it != UserConnMap.end()) { //toid offline
            it->second->send(js.dump());
            return;
        }
    }
    User user = userModel.query(toid);
    if (user.getState() == "online") {
        redis.publish(toid, js.dump());
        return;
    }
    offlineMsgModel.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr& conn, json& js) {
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr& conn, json& js) {
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    Group group(-1, name, desc);
    if (groupModel.createGroup(group)) {
        groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js) {
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(ConnMutex);
    for (int id:useridVec) {
        auto it = UserConnMap.find(id);
        if (it != UserConnMap.end()) {
            it->second->send(js.dump());
        }
        else {
            User user = userModel.query(id);
            if (user.getState() == "online") {
                redis.publish(id, js.dump());
            }
            else {
                offlineMsgModel.insert(id, js.dump());
            }     
        }
    }
}

void ChatService::handleRedisSubscribeMessage(int userid, string msg) {
    json js = json::parse(msg.c_str());
    lock_guard<mutex> lock(ConnMutex);
    auto it = UserConnMap.find(userid);
    if (it != UserConnMap.end()) {
        it->second->send(js.dump());
        return;
    }  
    offlineMsgModel.insert(userid, msg);
}