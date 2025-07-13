mprpc分布式网络通信框架项目和测试源码，技术栈c++、RPC远程调用原理、Protobuf数据序列化和反序列化、muduo网络库编程、异步日志等

编译方式 方式一： ./autobuild.sh 方式二： cd build rm -rf * cmake .. make

运行方式  cd bin ./provider -i test.conf ./consumer -i test.conf 
