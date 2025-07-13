#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"
int main(int argc, char **argv)
{

    // 初始化
    MprpcApplication::Init(argc, argv);

    // Login调用演示
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel);
    // rpc请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // rpc响应
    fixbug::LoginResponse response;
    // 发起rpc方法调用,同步rpc调用
    stub.Login(nullptr, &request, &response, nullptr);

    // rpc调用完成，读结果
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
    }

    // Register调用演示
    // rpc请求参数
    fixbug::RegisterRequest req;
    req.set_id(10);
    req.set_name("dfw");
    req.set_pwd("987654321");
    // rpc响应
    fixbug::RegisterResponse rsp;
    // 发起rpc方法调用,同步rpc调用
    stub.Register(nullptr, &req, &rsp, nullptr);

    // rpc调用完成，读结果
    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc Register response:" << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc Register response error:" << rsp.result().errmsg() << std::endl;
    }
    return 0;
}