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

static int SetNonBlock(int iSock)
{
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}

int main(){

    int ret = pipe(m_tickles);
	int fg = fcntl(m_tickles[0],F_GETFL,0);
	fg |= O_NONBLOCK;
	fcntl(m_tickles[0],F_SETFL,fg);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"pipe no success errno : "<< errno 
								   << " strerr : " << strerror(errno);
		throw std::logic_error("pipe no success");
	}

    return 0;
}
