#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察器,zkserver给zkclient的通知
void global_wather(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 回调消息类型（会话相关）
    {
        if (state == ZOO_CONNECTED_STATE) // 连接成功
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        // 关闭句柄，释放资源
        zookeeper_close(m_zhandle);
    }
}
void ZkClient::Start()
{
    std::string host = MprpcApplication::getInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::getInstance().GetConfig().Load("zookeeperport");
    std::string constr = host + ":" + port;

    m_zhandle = zookeeper_init(constr.c_str(), global_wather, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buf[128];
    int bufferlen = sizeof(path_buf);
    int flag;
    // 判断是否存在
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) // 不存在
    {
        // 创建
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buf, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "znode create success...path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error...path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据路径，获取指定节点值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        std::cout << "get znode error...path" << path << std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}
