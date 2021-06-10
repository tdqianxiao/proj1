#ifndef __TADPOLE_TCP_SERVER_H__
#define __TADPOLE_TCP_SERVER_H__

#include "co_routine.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#ifdef __FreeBSD__
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "address.h"

namespace tadpole{
    class TcpServer:public std::enable_shared_from_this<TcpServer>{
    public:
        typedef std::shared_ptr<TcpServer> ptr; 

        struct task_t{
            int fd;
            stCoRoutine_t *co;
            TcpServer::ptr server;
        };
    public:
        TcpServer(int ant = 1024);
        virtual ~TcpServer();
    public:
        void init();
        void start();

        void push(task_t * task);
        bool empty();
        task_t * erase();

        /**
         * @brief 响应fd事件句柄
         * @param[in] fd 文件描述符
         * @return -1 关闭客户端，0不关闭
         */
        virtual int handle(int fd);
    public:
        static int SetNonBlock(int iSock);
    private:
        int bind(Address::ptr addr);
        void startReadWrite();
        void startAccept(int fd);
        int CSByIP(const std::string & ip,uint16_t port);
        int CSByUnixAddr(const std::string & path); 

        int initPipe();      
    private:
        int m_ant = 0;                      //最大同时处理任务数量
        bool m_flag;                        //是否绑定成功
        std::stack<task_t*> m_readWrite;
        std::vector<int> m_listenFd;
    };
}

#endif 