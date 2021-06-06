#include <iostream>
#include "src/luaState.h"
#include <assert.h>
#include <string>
#include "src/singleton.hpp"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static const int MAX_EVENT_SIZE = 256;
int xxx(int port){
	int flag = 1;
	int sock = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(port);
	inet_pton(AF_INET,"192.168.5.111",&server_sockaddr.sin_addr.s_addr);
	socklen_t server_len = sizeof(server_sockaddr);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	assert(0 == bind(sock, (struct sockaddr*)&server_sockaddr, server_len) );
	listen(sock,10);

	int epfd = epoll_create(2000);
	if(epfd == -1){
		return -1; 
	}
	epoll_event * events = new epoll_event[MAX_EVENT_SIZE];
	std::shared_ptr<epoll_event> deleter(events,[](epoll_event * ptr){
			delete[] ptr;
		});

	epoll_event event ; 
	memset(&event, 0 , sizeof(event));
	event.events =  EPOLLIN;  
	event.data.fd = sock;
	epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&event);

	while(1){
		int ret = epoll_wait(epfd,events,MAX_EVENT_SIZE,-1);
		if(ret == -1){
			return -1;
		}else if(ret == 0){
			continue; 
		}
	
		char buf[1024] = {};
		for(int i= 0 ; i < ret ; ++i){
			//std::cout<<"fd:"<<events[i].data.fd<<std::endl;
			if (sock == events[i].data.fd){
				int fd = accept(sock,nullptr,nullptr);
				epoll_event xxx ; 
				memset(&xxx, 0 , sizeof(xxx));
				xxx.events = EPOLLIN; 
				xxx.data.fd = fd;
				epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&xxx);
			}else{
				std::cout<<111<<std::endl;
				int ret = read(events[i].data.fd,&buf,sizeof(buf));
				std::cout<<events[i].data.fd<<":    "<<buf << "    ret:" << ret <<std::endl;
			}
		}
	}
	
}	

int main (int argv,char ** argc){
	tadpole::luaState::ptr lua(new tadpole::luaState);
	lua->run();
	return 0;	
}


