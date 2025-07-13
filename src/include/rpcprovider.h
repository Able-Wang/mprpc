#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Callbacks.h>
#include <google/protobuf/descriptor.h>
#include <string>
#include <functional>
#include <unordered_map>

// 框架提供专门服务发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 框架提供给外部接口，发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程调用服务
    void Run();

private:
    muduo::net::EventLoop m_eventLoop;

    // 服务信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_method;
    };
    // 注册成功的服务对象和服务方法信息
    std::unordered_map<std::string, ServiceInfo> m_ServiceMap;

    // Socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    // 消息回调
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    // Closure回调函数，用于序列化rpc响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message *);
};
