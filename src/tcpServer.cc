#include "co_routine.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "tcpServer.h"
#include "config.h"
#include "log.h"

namespace tadpole{
    struct AddrInfo{
        std::string ip = "";
        uint16_t port = 0;
        std::string unix = ""; 

        std::string toString(){
            std::stringstream ss;
            if(!ip.empty()){
                ss << "["<<ip<<"]:"<<port;
            }
            if(!unix.empty()){
                ss << unix;
            }
            return ss.str();
        }
    };

    template <>
    class LexicalCast<std::string,AddrInfo>{
    public:
        AddrInfo operator()(const std::string & str){
            AddrInfo addr;
            YAML::Node node = YAML::Load(str);
            if(node["ip"].IsDefined()){
                addr.ip = node["ip"].as<std::string>();
            }
            if(node["port"].IsDefined()){
                addr.port = node["port"].as<uint16_t>();
            }
            if(node["unix"].IsDefined()){
                addr.unix = node["unix"].as<std::string>();
            }
            return addr;
        }
    };

    template <>
    class LexicalCast<AddrInfo,std::string>{
    public:
        std::string operator()(const AddrInfo& addr){
            YAML::Node node; 
            if(!addr.ip.empty()){
                node["ip"] = addr.ip;
                node["port"] = std::to_string(addr.port);
            }
            if(!addr.unix.empty()){
                node["unix"] = addr.unix;
            }
            std::stringstream ss; 
            ss << node;
            return ss.str();
        }
    };

    struct fd_ctx{
        int fd;
        stCoRoutine_t *co;
        TcpServer::ptr server;
    };

    static Logger::ptr logger = TADPOLE_FIND_LOGGER("system");

    static ConfigVar<std::vector<AddrInfo> >::ptr g_ip_info = 
        Config::Lookup<std::vector<AddrInfo> >("tcp.address",std::vector<AddrInfo>(),"server bind ip or unixaddr");

    class TcpServerIniter{
    public:
        TcpServerIniter(){
            Config::LoadFromYaml("./config/yaml/addr.yml");
        }
    };
    TcpServerIniter __tcpServerIniter;

    int SetNonBlock(int iSock)
    {
        int iFlags;
        iFlags = fcntl(iSock, F_GETFL, 0);
        iFlags |= O_NONBLOCK;
        iFlags |= O_NDELAY;
        int ret = fcntl(iSock, F_SETFL, iFlags);
        return ret;
    }

    void TcpServer::handle(fd_ctx * ctx){
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
            return ; 
        }
    }

    void* write_start(void * arg){ 
        co_enable_hook_sys();
        fd_ctx * ctx = (fd_ctx*) arg;
        std::shared_ptr<int> cb(new int,[ctx](void * ptr){
            close(ctx->fd);
            free(ptr);
        });
        ctx->server->handle(ctx);
        return nullptr; 
        // char buf[1024] = {};
        // for(;;){
        //     memset(buf,0,sizeof(buf));
        //     int size = read(ctx->fd,buf,1024);
        //     //TADPOLE_LOG_DEBUG(logger)<<"read size : "<<size<<" errno"<<errno<<EAGAIN;
        //     if(size > 0 ){
        //         TADPOLE_LOG_DEBUG(logger)<< buf ;
        //         continue;
        //     }else if (size == 0){
        //     }else{
        //         if(errno == EAGAIN){
        //             struct pollfd pf = { 0 };
        //             pf.fd = ctx->fd;
        //             pf.events = (POLLIN|POLLERR|POLLHUP);
        //             co_poll( co_get_epoll_ct(),&pf,1,1000);
        //             continue; 
        //         }
        //     }
        //     return nullptr; 
        // }
    }

    void * accept_start(void* arg){
        co_enable_hook_sys();
        fd_ctx * ctx = (fd_ctx*)arg;
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
            fd_ctx* cctx = (fd_ctx *)calloc(1,sizeof(fd_ctx));
            stCoRoutine_t * co = nullptr; 
            cctx->fd = fd ;
            cctx->co = co ;
            cctx->server = ctx->server;
            co_create(&co,nullptr,write_start,cctx);
            co_resume(co);
        }
        return nullptr;
    }

    int checkUnix(const std::string & path){
        return 0;
    }

    int checkIP(const std::string & ip,uint16_t port){
        return 0;
    }
   

    int TcpServer::ServerInit(Address::ptr addr){
        assert(addr);
        int fd = socket(addr->getFamily(),SOCK_STREAM,0);
        if(fd == -1){
            //TODO error 
            TADPOLE_LOG_ERROR(logger)<<"create socket errno : "<<errno<< " errstr: "<<strerror(errno);
            return -1;
        }
        int flag = 1 ;
        setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
        int ret = SetNonBlock(fd);
        if(ret == -1){
            //TODO error 
            TADPOLE_LOG_ERROR(logger)<<"fcntl fd errno : "<<errno<< " errstr: "<<strerror(errno);
            return -1;
        }
        ret = bind(fd,addr->getAddr(),addr->getAddrLen());
        if(ret == -1){
            //TODO error 
            TADPOLE_LOG_ERROR(logger)<<"bind fd errno : "<<errno<< " errstr: "<<strerror(errno);
            close(fd);
            return -1; 
        }
        ret = listen(fd,1024);
        if(ret == -1){
            //TODO error 
            TADPOLE_LOG_ERROR(logger)<<"listen fd errno : "<<errno<< " errstr: "<<strerror(errno);
            close(fd);
            return -1;
        }
        TADPOLE_LOG_ERROR(logger)<<"bind & listen : "<<addr->toString()<< " successful !";
        //accept
        fd_ctx * ctx = (fd_ctx *)calloc(1,sizeof(fd_ctx));
        stCoRoutine_t * co = nullptr; 
        co_create(&co,nullptr,accept_start,ctx);
        ctx->fd = fd; 
        ctx->co = co;
        ctx->server = this->shared_from_this();
        co_resume(co);
        return fd; 
    }

    TcpServer::TcpServer(){
        
    }

    TcpServer::~TcpServer(){

    }

    int TcpServer::CSByUnixAddr(const std::string & path){
        if(checkUnix(path) != 0){
            TADPOLE_LOG_ERROR(logger)<<"unix path: "<<path<<" error !";
            return -1;
        }
        int ret = remove(path.c_str());
        if(ret == 0){
            TADPOLE_LOG_ERROR(logger)<<"Remove: "<<path;
        }
        Address::ptr addr(new UnixAddress(path));
        if(ServerInit(addr) == -1){
            return -1;
        }
        TADPOLE_LOG_ERROR(logger)<<addr->toString()<<" start service !";
        return 0;
    }

    int TcpServer::CSByIP(const std::string & ip,uint16_t port){
        if(checkIP(ip,port) != 0){
            TADPOLE_LOG_ERROR(logger)<<"ip and port: "<<ip<<":"<<port<<" error !";
            return -1;
        }
        Address::ptr addr = IPv4Address::Create(ip.c_str(),port);
         if(ServerInit(addr) == -1){
            TADPOLE_LOG_ERROR(logger)<<"init "<<addr->toString()<<" error !";
            return -1;
        }
        TADPOLE_LOG_ERROR(logger)<<addr->toString()<<" start service !";
        return 0;
    }

    void TcpServer::start(){
        std::vector<AddrInfo> info = g_ip_info->getValue();
        bool flag = false;
        for(auto & it : info){
            if(!it.ip.empty() && it.port != 0){
                if(CSByIP(it.ip,it.port) == 0){
                    flag = true ;
                }
            }
            if(!it.unix.empty()){
                if(CSByUnixAddr(it.unix)){
                    flag = true ;
                }
            }
        }
        if(flag == true){
            TADPOLE_LOG_DEBUG(logger)<<"start server success!";
            co_eventloop(co_get_epoll_ct(),0,0);
        }
    }
}