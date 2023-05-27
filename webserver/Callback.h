#ifndef CALLBACK_H
#define CALLBACK_H

#include <functional>
#include <memory>
#include "Buffer.h"
class TcpConnection;
using namespace std;

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using ConnectionCallback = function<void(const TcpConnectionPtr&)>;
using MessageCallback = function<void(const TcpConnectionPtr&, Buffer*)>;
using WriteCompleteCallback = function<void(const TcpConnectionPtr&)>;
using CloseCallback = function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = function<void(const TcpConnectionPtr&, size_t)>;

#endif