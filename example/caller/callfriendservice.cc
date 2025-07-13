#include <iostream>
#include "friend.pb.h"
#include "mprpcapplication.h"

int main(int argc, char **argv)
{

    // 初始化
    MprpcApplication::Init(argc, argv);

    // Login调用演示
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel);
    // rpc请求参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    // rpc响应
    fixbug::GetFriendsListResponse response;
    // 发起rpc方法调用,同步rpc调用
    MprpcController controller;
    stub.GetFriendsLists(&controller, &request, &response, nullptr);

    // rpc调用完成，读结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc GetFriendsList response success!" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; ++i)
            {
                std::cout << "index:" << (i + 1) << " name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendsList response error:" << response.result().errmsg() << std::endl;
        }
    }
}