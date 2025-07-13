#include "rpcprovider.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "logger.h"
#include "zookeeperutil.h"
// 框架提供给外部接口，发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo serviceinfo;

    // 获取服务描述对象
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名称
    std::string service_name = pserviceDesc->name();
    // 获取服务service方法数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service_name:" << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取对应索引方法
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        serviceinfo.m_method.insert({method_name, pmethodDesc});

        // std::cout << "method_name:" << method_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    serviceinfo.m_service = service;
    m_ServiceMap.insert({service_name, serviceinfo});
}

// 启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::getInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定连接回调和消息读写函数
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo线程数量
    server.setThreadNum(4);

    // 把当前rpc节点发布服务注册到zk上面，让rpc_client可以从zk上发现服务
    // session timeout 30s，1/3timeout发送ping信息
    ZkClient zkCli;
    zkCli.Start();
    for (auto &sp : m_ServiceMap)
    {
        // /serice_name
        std::string service_path = "/" + sp.first;
        // 永久节点
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_method)
        {
            // /serice_name/method_name
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // 临时节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动
    server.start();
    m_eventLoop.loop();
}

// 连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

// 字符流 header_size(4字节)+header_str+args_str
// 消息回调
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
    // 网络接收字符流
    std::string recv_str = buffer->retrieveAllAsString();

    // 读取4字节长度，获取头长度
    uint32_t header_size = 0;
    recv_str.copy((char *)&header_size, 4, 0);

    // 根据header_size读取头，并进行反序化
    std::string rpc_header_str = recv_str.substr(4, header_size);
    mprpc::RpcHeader rpc_header;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    // 数据头反序列成功
    if (rpc_header.ParseFromString(rpc_header_str))
    {
        service_name = rpc_header.service_name();
        method_name = rpc_header.method_name();
        args_size = rpc_header.args_size();
    }
    // 数据头反序列失败
    else
    {
        // std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        LOG_ERR("rpc_header_str:%s", rpc_header_str.c_str());
        return;
    }

    // 解析获取参数字符流
    std::string args_str = recv_str.substr(4 + header_size, recv_str.size());

    // 打印调试信息
    // std::cout << "======================================================" << std::endl;
    // std::cout << "header_size:" << header_size << std::endl;
    // std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    // std::cout << "service_name:" << service_name << std::endl;
    // std::cout << "method_name:" << method_name << std::endl;
    // std::cout << "args_size:" << args_size << std::endl;
    // std::cout << "args_str:" << args_str << std::endl;
    // std::cout << "======================================================" << std::endl;

    // 获取service对象和method方法
    auto it = m_ServiceMap.find(service_name);
    if (it == m_ServiceMap.end())
    {
        // std::cout << service_name << " is not exist!" << std::endl;
        LOG_ERR("%s is not exist! ", service_name.c_str());
        return;
    }

    auto mit = it->second.m_method.find(method_name);
    if (mit == it->second.m_method.end())
    {
        // std::cout << method_name << " is not exist!" << std::endl;
        LOG_ERR("%s is not exist! ", method_name.c_str());
        return;
    }

    google::protobuf::Service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成rpc方法调用的request和response
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        // std::cout << "request parse error content:" << rpc_header_str << std::endl;
        LOG_ERR("request parse error content::%s", rpc_header_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 创建method回调
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &RpcProvider::SendRpcResponse,
                                                                                                 conn, response);
    // 根据远端rpc请求，调用当前rpc节点发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure回调函数，用于序列化rpc响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        // 发送给rpc调用方
        conn->send(response_str);
    }
    else
    {
        // std::cout << "serialize response_str error" << std::endl;
        LOG_ERR("serialize response_str error");
    }
    conn->shutdown(); // 短连接，主动断开连接
}