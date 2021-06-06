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

static void SetAddr(const char *pszIP,const unsigned short shPort,struct sockaddr_in &addr)
{
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(shPort);
	int nIP = 0;
	if( !pszIP || '\0' == *pszIP   
	    || 0 == strcmp(pszIP,"0") || 0 == strcmp(pszIP,"0.0.0.0") 
		|| 0 == strcmp(pszIP,"*") 
	  )
	{
		nIP = htonl(INADDR_ANY);
	}
	else
	{
		nIP = inet_addr(pszIP);
	}
	addr.sin_addr.s_addr = nIP;
}

int g_sock = -1; 

struct xxx_t{
    int fd;
    stCoRoutine_t *co;
};

void* my_write(void * arg){ 
    co_enable_hook_sys();
    xxx_t * xxx = (xxx_t*) arg;
    char buf[1024] = {};
    for(;;){
        memset(buf,0,sizeof(buf));
        int size = read(xxx->fd,buf,1024);
        //TADPOLE_LOG_DEBUG(logger)<<"read size : "<<size<<" errno"<<errno<<EAGAIN;
        if(size > 0 ){
            TADPOLE_LOG_DEBUG(logger)<< buf ;
            continue;
        }else if (size == 0){
        }else{
            if(errno == EAGAIN){
                struct pollfd pf = { 0 };
                pf.fd = xxx->fd;
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
    for(;;){
        //TADPOLE_LOG_DEBUG(logger)<<"accept start !";
        int fd = accept(g_sock,nullptr,nullptr);
        //TADPOLE_LOG_DEBUG(logger)<<"accept fd : "<<fd;
        if(fd == -1){
            struct pollfd pf = { 0 };
			pf.fd = g_sock;
			pf.events = (POLLIN|POLLERR|POLLHUP);
			co_poll( co_get_epoll_ct(),&pf,1,1000 );
			continue;
        }
        SetNonBlock(fd);
        xxx_t* xxx = (xxx_t *)calloc(1,sizeof(xxx_t));
        stCoRoutine_t * co = nullptr; 
        xxx->fd = fd ;
        xxx->co = co ;
        co_create(&co,nullptr,my_write,xxx);
        co_resume(co);
    }

    return nullptr; 
}


int main(int argc,char *argv[])
{
    int fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(fd == -1){
        return -1; 
    }
    g_sock = fd;
    int flag = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    SetNonBlock(fd);
    sockaddr_in addr;
    SetAddr("127.0.0.1",9000,addr);
    int ret = bind(fd,(sockaddr*)&addr,sizeof(addr));
    if(ret == -1){
        return -1; 
    }

    ret = listen(fd,1024);
    if(ret == -1){
        return -1; 
    }

    TADPOLE_LOG_DEBUG(logger)<<"bind : 127.0.0.1:9000";

    stCoRoutine_t * co = nullptr; 
    co_create(&co,nullptr,my_accept,nullptr);
    co_resume(co);

   	co_eventloop( co_get_epoll_ct(),0,0 );
}

