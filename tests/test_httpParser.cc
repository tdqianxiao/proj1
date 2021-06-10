
#include "src/tcpServer.h"
#include "src/log.h"
#include "src/http/http_request.h"

using namespace tadpole;
tadpole::Logger::ptr logger = TADPOLE_FIND_LOGGER("root");

class EchoServer:public TcpServer{
public:
    typedef std::shared_ptr<EchoServer> ptr; 
public:
    int handle(int fd){
        char buf[256] = {0};
        int more = 0; 
        HttpRequestParser::ptr parser(new HttpRequestParser);
        for(;;){
            memset(buf + more,0,sizeof(buf) - more);
            int ret = read( fd,buf + more,sizeof(buf) - more);
            if( ret > 0 ){
                int off = parser->execute(buf,ret + more);
                more = sizeof(buf) - off;
                if(parser->isFinish()){
                    HttpRequest::ptr req = parser->getRequest();
                    TADPOLE_LOG_INFO(logger) << req->toString();
                    return -1;
                }
            }
            if( ret <= 0 ){
                if (errno == EAGAIN){
                    return 0;
                }
                return -1;
            }
        }
    }
};

int main(int argc,char *argv[])
{ 
    TcpServer::ptr server(new EchoServer());
    server->start();
}
