
#include "co_routine.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "src/address.h"
#include "src/log.h"

#ifdef __FreeBSD__
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#endif

tadpole::Logger::ptr logger = TADPOLE_FIND_LOGGER("root");

static int SetNonBlock(int iSock)
{
    int iFlags;
    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

int g_sock = -1; 

struct fd_ctx{
    int fd;
    stCoRoutine_t *co;
};

void* my_write(void * arg){ 
    co_enable_hook_sys();
    fd_ctx * ctx = (fd_ctx*) arg;
    char buf[1024] = {};
    for(;;){
        memset(buf,0,sizeof(buf));
        int size = read(ctx->fd,buf,1024);
        //TADPOLE_LOG_DEBUG(logger)<<"read size : "<<size<<" errno"<<errno<<EAGAIN;
        if(size > 0 ){
            TADPOLE_LOG_DEBUG(logger)<< buf ;
            continue;
        }else if (size == 0){
        }else{
            if(errno == EAGAIN){
                struct pollfd pf = { 0 };
                pf.fd = ctx->fd;
                pf.events = (POLLIN|POLLERR|POLLHUP);
                co_poll( co_get_epoll_ct(),&pf,1,1000);
                continue; 
            }
        }
        return nullptr; 
    }
}

void * my_accept(void * arg){
    co_enable_hook_sys();

    fd_ctx *ctx = (fd_ctx *)arg;
    for(;;){
        struct pollfd pf = { 0 };
        pf.fd = ctx->fd;
        pf.events = (POLLIN|POLLERR|POLLHUP);

		co_poll( co_get_epoll_ct(),&pf,1,1000);
        int fd = accept(ctx->fd,nullptr,nullptr);
        if(fd == -1){
			continue;
        }
        SetNonBlock(fd);
        fd_ctx* xxx = (fd_ctx *)calloc(1,sizeof(fd_ctx));
        stCoRoutine_t * co = nullptr; 
        xxx->fd = fd ;
        xxx->co = co ;
        co_create(&co,nullptr,my_write,xxx);
        co_resume(co);
    }

    return nullptr; 
}
#include <memory>
#include <assert.h>

using namespace tadpole;
int CreateTcpSocket(IPAddress::ptr addr){
    assert(addr);
    int fd = socket(addr->getFamily(),SOCK_STREAM,IPPROTO_TCP);
    if(fd == -1){
        //TODO error 
        TADPOLE_LOG_DEBUG(logger)<<"create socket errno : "<<errno<< " errstr: "<<strerror(errno);
        return -1;
    }
    int flag = 1 ;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    int ret = SetNonBlock(fd);
    if(ret == -1){
        //TODO error 
        TADPOLE_LOG_DEBUG(logger)<<"fcntl fd errno : "<<errno<< " errstr: "<<strerror(errno);
        return -1;
    }
    ret = bind(fd,addr->getAddr(),addr->getAddrLen());
    if(ret == -1){
        //TODO error 
        TADPOLE_LOG_DEBUG(logger)<<"bind fd errno : "<<errno<< " errstr: "<<strerror(errno);
        close(fd);
        return -1; 
    }
    ret = listen(fd,1024);
    if(ret == -1){
        //TODO error 
        TADPOLE_LOG_DEBUG(logger)<<"listen fd errno : "<<errno<< " errstr: "<<strerror(errno);
        close(fd);
        return -1;
    }
    TADPOLE_LOG_DEBUG(logger)<<"bind & listen : "<<addr->toString()<< " successful !";
    //accept
    fd_ctx * ctx = (fd_ctx *)calloc(1,sizeof(fd_ctx));
    stCoRoutine_t * co = nullptr; 
    co_create(&co,nullptr,my_accept,ctx);
    ctx->fd = fd; 
    ctx->co = co;
    co_resume(co);
    return 0; 
}

int main(int argc,char *argv[])
{
    std::multimap<std::string,tadpole::Address::ptr> ips;
    tadpole::Address::LookupInterfaces(ips);

    for(auto & it: ips){
        if(it.second->getFamily() == AF_INET){
            IPAddress::ptr addr = nullptr; 
            try{
                addr = std::dynamic_pointer_cast<IPAddress>(it.second);
                addr->setPort(9000);
            }catch(...){
                TADPOLE_LOG_DEBUG(logger)<<"dynamic_pointer_cast addr : "<<addr->toString()<< " error !";
            }
            int ret = CreateTcpSocket(addr);
            if(ret == -1){
                return -1; 
            }
        }
    }

   	co_eventloop( co_get_epoll_ct(),0,0 );
}


// int main (int argv,char ** argc){
//     std::multimap<std::string,tadpole::Address::ptr> ips;
//     tadpole::Address::LookupInterfaces(ips);    
//     for(auto & it : ips){
//         TADPOLE_LOG_INFO(logger)<<it.first;
//         TADPOLE_LOG_INFO(logger)<<it.second->toString();
//     }
// 	return 0;	
// }
