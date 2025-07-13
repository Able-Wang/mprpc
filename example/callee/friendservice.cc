#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t id)
    {
        std::cout << "do local  GetFriendsList service! userid:" << id << std::endl;
        std::vector<std::string> name;
        name.push_back("wang yi");
        name.push_back("li si");
        name.push_back("Able");
        return name;
    }

    virtual void GetFriendsLists(::google::protobuf::RpcController *controller,
                                 const ::fixbug::GetFriendsListRequest *request,
                                 ::fixbug::GetFriendsListResponse *response,
                                 ::google::protobuf::Closure *done)
    {
        //
        uint32_t id = request->userid();
        //
        std::vector<std::string> res = GetFriendsList(id);
        //
        fixbug::Resultcode *rsc = response->mutable_result();
        rsc->set_errcode(0);
        rsc->set_errmsg("");

        for (std::string &name : res)
        {
            std::string *p = response->add_friends();
            *p = name;
        }

        // 回调
        done->Run();
    }
};

int main(int argc, char **argv)
{

    // 框架初始化操作
    MprpcApplication::Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new FriendService);

    provider.Run();

    return 0;
}