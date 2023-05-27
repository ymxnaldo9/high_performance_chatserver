#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include "webserver/TcpConnection.h"
#include "utils/json.hpp"
#include "utils/redis.h"
#include <unordered_map>
#include <functional>
#include <mutex>
#include "model/UserModel.h"
#include "model/OfflineMessageModel.h"
#include "model/FriendModel.h"
#include "model/GroupModel.h"
using namespace std;
using json = nlohmann::json;
using MsgHandler = function<void(const TcpConnectionPtr& conn, json& js)>;

class ChatService {
public:
    static ChatService* instance();
    void login(const TcpConnectionPtr& conn, json& js);
    void loginout(const TcpConnectionPtr& conn, json& js);
    void reg(const TcpConnectionPtr& conn, json& js);
    void oneChat(const TcpConnectionPtr& conn, json& js);
    void addFriend(const TcpConnectionPtr& conn, json& js);
    void createGroup(const TcpConnectionPtr& conn, json& js);
    void addGroup(const TcpConnectionPtr& conn, json& js);
    void groupChat(const TcpConnectionPtr& conn, json& js);
    void reset();
    MsgHandler getHandler(int msgid);
    void clientCloseException(const TcpConnectionPtr& conn);
    void handleRedisSubscribeMessage(int userid, string msg);
private:
    ChatService();
    unordered_map<int, MsgHandler> MsgHandlerMap;
    unordered_map<int, TcpConnectionPtr> UserConnMap;
    UserModel userModel;
    OfflineMsgModel offlineMsgModel;
    FriendModel friendModel;
    GroupModel groupModel;
    mutex ConnMutex;
    Redis redis;
};

#endif