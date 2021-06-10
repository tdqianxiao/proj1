#include "tcpServer.h"
#include "config.h"
#include "log.h"
#include "timer.h"

int co_accept(int fd, struct sockaddr *addr, socklen_t *len );

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

    static Logger::ptr logger = TADPOLE_FIND_LOGGER("system");

    static ConfigVar<std::vector<AddrInfo> >::ptr g_ip_info = 
        Config::Lookup<std::vector<AddrInfo> >("tcp.address",std::vector<AddrInfo>(),"server bind ip or unixaddr");

    struct IpInfoIniter{
        IpInfoIniter(){
            Config::LoadFromYaml("./config/yaml/addr.yml");
        }
    };
    static IpInfoIniter __ipInfoIniter;

    static void *readwrite_routine( void *arg ){
        co_enable_hook_sys();
        TcpServer::task_t *co = (TcpServer::task_t*)arg;
        for(;;){
            if( -1 == co->fd ){
                co->server->push( co );
                co_yield_ct();
                continue;
            }
            int fd = co->fd;
            co->fd = -1;
            for(;;){
                struct pollfd pf = { 0 };
                pf.fd = fd;
                pf.events = (POLLIN|POLLERR|POLLHUP);
                co_poll( co_get_epoll_ct(),&pf,1,1000);

                int ret = co->server->handle(fd);
                if(ret == -1){
                    close( fd );
                    break; 
                }
            }
        }
        return 0;
    }

    static void *accept_routine( void * arg){
        co_enable_hook_sys();
        TcpServer::task_t * ctx = (TcpServer::task_t*)arg;
        for(;;){
            if( ctx->server->empty() ){
                struct pollfd pf = { 0 };
                pf.fd = -1;
                poll( &pf,1,1000);
                continue;
            }
            int fd = co_accept(ctx->fd,nullptr,nullptr);
            if( fd < 0 ){
                struct pollfd pf = { 0 };
                pf.fd = ctx->fd;
                pf.events = (POLLIN|POLLERR|POLLHUP);
                co_poll( co_get_epoll_ct(),&pf,1,1000 );
                continue;
            }
            if( ctx->server->empty() ){
                close( fd );
                continue;
            }
            TcpServer::SetNonBlock( fd );
            TcpServer::task_t *co = ctx->server->erase();
            co->fd = fd;
            co_resume( co->co );
        }
        return 0;
    }

    TcpServer::TcpServer(int ant)
        :m_ant(ant){
        init();
    }

    TcpServer::~TcpServer(){

    }

    int TcpServer::handle(int fd){
        char buf[1024*128] = {0};
        int ret = read( fd,buf,sizeof(buf) );
        if( ret > 0 ){
            ret = write( fd,buf,ret );
        }
        if( ret <= 0 ){
            if (errno == EAGAIN){
                return 0;
            }
            return -1;
        }
        return 0; 
    }

    void TcpServer::init(){
        std::vector<AddrInfo> info = g_ip_info->getValue();
        for(auto & it : info){
            if(!it.ip.empty() && it.port != 0){
                int fd = CSByIP(it.ip,it.port);
                if(fd != -1){
                    m_listenFd.push_back(fd);
                    m_flag = true ;
                }
            }
            if(!it.unix.empty()){
                int fd = CSByUnixAddr(it.unix);
                if(fd != -1){
                    m_listenFd.push_back(fd);
                    m_flag = true ;
                }
            }
        }
    }

    void TcpServer::start(){
        if(m_flag){
            //开启readwrite任务
            startReadWrite();
            //开启accept任务
            for(auto & it : m_listenFd){
                startAccept(it);
            }
            TADPOLE_LOG_INFO(logger)<<"start service";
            //进入epoll循环
            co_eventloop( co_get_epoll_ct(),0,0 );
        }
    }

    void TcpServer::startReadWrite(){
        for(int i=0 ; i<m_ant ; i++){
			task_t * task = (task_t*)calloc( 1,sizeof(task_t) );
			task->fd = -1;
            task->server = this->shared_from_this();
			co_create( &(task->co),nullptr,readwrite_routine,task );
			co_resume( task->co );
		}
    }

    void TcpServer::startAccept(int fd){
        stCoRoutine_t *accept_co = nullptr;
        TcpServer::task_t * ctx = (TcpServer::task_t *)calloc(1,sizeof(TcpServer::task_t));
		co_create( &accept_co,nullptr,accept_routine,ctx);

        ctx->fd = fd;
        ctx->co = accept_co;
        ctx->server = this->shared_from_this();

		co_resume( accept_co );
    }

    void TcpServer::push(TcpServer::task_t * task){
        m_readWrite.push(task);
    }

    bool TcpServer::empty(){
        return m_readWrite.empty();
    }

    TcpServer::task_t * TcpServer::erase(){
        task_t * task = m_readWrite.top();
        m_readWrite.pop();
        return task;
    }

    int checkUnix(const std::string & path){
        return 0; 
    }

    int checkIP(const std::string & ip,uint16_t port){
        return 0;
    }

    int TcpServer::CSByUnixAddr(const std::string & path){
        if(checkUnix(path) != 0){
            TADPOLE_LOG_ERROR(logger)<<"unix path: "<<path<<" error !";
            return -1;
        }
        remove(path.c_str());
        Address::ptr addr(new UnixAddress(path));
        int fd = bind(addr);
        if(fd == -1){
            return -1;
        }
        return fd;
    }

    int TcpServer::CSByIP(const std::string & ip,uint16_t port){
        if(checkIP(ip,port) != 0){
            TADPOLE_LOG_ERROR(logger)<<"ip and port: "<<ip<<":"<<port<<" error !";
            return -1;
        }
        Address::ptr addr = IPv4Address::Create(ip.c_str(),port);
        int fd = bind(addr);
        if(fd == -1){
            TADPOLE_LOG_ERROR(logger)<<"init "<<addr->toString()<<" error !";
            return -1;
        }
        return fd;
    }

    int TcpServer::SetNonBlock(int iSock){
        int iFlags;
        iFlags = fcntl(iSock, F_GETFL, 0);
        iFlags |= O_NONBLOCK;
        iFlags |= O_NDELAY;
        int ret = fcntl(iSock, F_SETFL, iFlags);
        return ret;
    }
        
    int TcpServer::bind(Address::ptr addr){
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
        ret = ::bind(fd,addr->getAddr(),addr->getAddrLen());
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
        TADPOLE_LOG_INFO(logger)<<"bind & listen : "<<addr->toString()<< " successful";
        return fd;
    }
}