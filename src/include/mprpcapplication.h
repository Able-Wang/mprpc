#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架基础类
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication &getInstance();
    MprpcConfig &GetConfig();

private:
    static MprpcConfig m_config;
    MprpcApplication() {}
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;
};