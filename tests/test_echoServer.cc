
#include "src/tcpServer.h"
#include "src/log.h"

using namespace tadpole;
tadpole::Logger::ptr logger = TADPOLE_FIND_LOGGER("root");

class EchoServer:public TcpServer{
public:
    typedef std::shared_ptr<EchoServer> ptr; 
public:
    int handle(int fd){
        char buf[1024*128] = {0};
        int ret = read( fd,buf,sizeof(buf) );
        if( ret > 0 ){
            TADPOLE_LOG_INFO(logger) << buf;
        }
        if( ret <= 0 ){
            if (errno == EAGAIN){
                return 0;
            }
            return -1;
        }
        return 0; 
    }
};

int main(int argc,char *argv[])
{ 
    TcpServer::ptr server(new EchoServer());
    server->start();
}
