#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "do local login service!" << std::endl;
        std::cout << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "do local Register service!" << std::endl;
        std::cout << "id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    // 重载基类虚函数
    void Login(google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {

        //
        std::string name = request->name();
        std::string pwd = request->pwd();
        //
        bool Loginresult = Login(name, pwd);
        //
        fixbug::Resultcode *rsc = response->mutable_result();
        rsc->set_errcode(0);
        rsc->set_errmsg("");
        response->set_success(Loginresult);

        // 回调
        done->Run();
    }

    virtual void Register(google::protobuf::RpcController *controller,
                          const ::fixbug::RegisterRequest *request,
                          ::fixbug::RegisterResponse *response,
                          ::google::protobuf::Closure *done)
    {

        //
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();
        //
        bool Registerresult = Register(id, name, pwd);
        //
        fixbug::Resultcode *rsc = response->mutable_result();
        rsc->set_errcode(0);
        rsc->set_errmsg("");
        response->set_success(Registerresult);

        // 回调
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 框架初始化操作
    MprpcApplication::Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new UserService);

    provider.Run();

    return 0;
}