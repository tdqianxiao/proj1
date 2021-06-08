#ifndef __TADPOLE_TCP_SERVER_H__
#define __TADPOLE_TCP_SERVER_H__

#include <string>
#include <memory>
#include <vector>
#include "address.h"

namespace tadpole{
    struct fd_ctx;  

    class TcpServer:public std::enable_shared_from_this<TcpServer>{
    public:
        typedef std::shared_ptr<TcpServer> ptr; 
    public:
        TcpServer();
        ~TcpServer();

        void start();

        virtual void handle(fd_ctx * ctx);
    private:
        int ServerInit(Address::ptr addr);
        int CSByUnixAddr(const std::string & path = "tmp/tadpole.sock");
        int CSByIP(const std::string & ip,uint16_t port);
    private:
        std::vector<int> m_acceptFd ;
    };

    struct fd_ctx{
        int fd;
        stCoRoutine_t *co;
        TcpServer::ptr server;
    };

}

#endif 