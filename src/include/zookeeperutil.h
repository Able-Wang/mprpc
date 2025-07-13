#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // 启动连接zkserver
    void Start();
    // 在zkserver根据指定路径创建znode
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数指定znode,获得节点值
    std::string GetData(const char *path);

private:
    // zk客户端句柄
    zhandle_t *m_zhandle;
};