#include <iostream>
#include "test.pb.h"
#include <string>
using namespace fixbug;

int main()
{
    // LoginRequest req;
    // req.set_name("zhang san");
    // req.set_pwd("123456");

    // std::string str;
    // if (req.SerializePartialToString(&str))
    // {
    //     std::cout << str.c_str() << std::endl;
    // }

    // LoginRequest reqb;
    // if (reqb.ParseFromString(str))
    // {
    //     std::cout << reqb.name() << std::endl;
    //     std::cout << reqb.pwd() << std::endl;
    // }

    GetFriendListsResponse res;
    Resultcode *rc = res.mutable_result();
    rc->set_errcode(0);

    User *user1 = res.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 = res.add_friend_list();
    user2->set_name("zhang san");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout << res.friend_list_size() << std::endl;

    return 0;
}