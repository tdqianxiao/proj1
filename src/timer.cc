#include "timer.h"
#include "utils.h"
#include "tcpServer.h"
#include "log.h"

#include <exception>

namespace tadpole{
    static Logger::ptr logger = TADPOLE_FIND_LOGGER("system");

    Timer::Timer(uint32_t ms,std::function<void()> cb,bool isForever)
        :m_cb(cb)
        ,m_isForever(isForever){
        uint32_t cur = GetTimeOfMS();
        m_begin = cur; 
        m_end = cur + ms;
    }

    Timer::Timer()
        :m_begin(GetTimeOfMS()){
    }

    int Timer::getMs(){
        int diff = m_end - m_begin;
        return (diff < 0) ? 0 : diff;
    }

    void Timer::cancel(){
        if(m_cb){
            m_cb();
            TimerMgr::GetInstance()->delTimer(this->shared_from_this());
        }
    }

    void Timer::remove(){
        TimerMgr::GetInstance()->delTimer(this->shared_from_this());
    }

    void Timer::reset(){
        uint32_t cur = GetTimeOfMS();
        uint32_t ms = m_end - m_begin;
        m_begin = cur; 
        m_end = cur + ms;
    }

    TimerManagers::TimerManagers(){
        int ret = init();
        if(ret == -1){
            throw std::logic_error("TimerManagers init error !");
        }
    }

    int TimerManagers::frontMs(){
        SetType::iterator front = m_timers.begin();
        if(front == m_timers.end()){
            return -1;
        }
        int ms = (*front)->getMs();
        return (ms < 0) ? 0 : ms; 
    }

    int TimerManagers::delTimer(Timer::ptr timer){
        SetType::iterator iter = m_timers.find(timer);
        if(iter == m_timers.end()){
            return -1;
        }
        m_timers.erase(iter);
        return 0;
    }

    Timer::ptr TimerManagers::addTimer(uint32_t ms,std::function<void()> cb,int isForever){
        Timer::ptr timer(new Timer(ms,cb));
        m_timers.insert(timer);
        return timer;
    }

    Timer::ptr TimerManagers::addTimerForever(uint32_t ms,std::function<void()> cb){
        return addTimer(ms,cb,true);
    }


    void TimerManagers::checkExpire(){
        std::vector<Timer::ptr> expireVec;
        expireVec.clear();
        Timer::ptr timer(new Timer());
        SetType::iterator end = m_timers.upper_bound(timer);

        for(auto iter = m_timers.begin(); iter != end; ++iter){
            expireVec.push_back(*iter);
        }
        m_timers.erase(m_timers.begin(),end);
        for(auto & it : expireVec){
            std::function<void()> cb = it->getCb();
            if(cb){
                cb();
            }
            if(it->isFor()){
                it->reset();
                m_timers.insert(it);
            }
        }
    }

    
    static void *timer_routine( void *arg ){
        co_enable_hook_sys();
        TcpServer::task_t *co = (TcpServer::task_t*)arg;
        for(;;){
            struct pollfd pf = { 0 };
            pf.fd = co->fd;
            pf.events = (POLLIN|POLLERR|POLLHUP);
            int time = TimerMgr::GetInstance()->frontMs();
            co_poll( co_get_epoll_ct(),&pf,1,time);
            TimerMgr::GetInstance()->checkExpire();
        }
        return 0;
    }

    int TimerManagers::init(){
        int ret = pipe(m_tickles);
        if(ret == -1){
            TADPOLE_LOG_ERROR(logger)<<"create pipe fatil ! ";
            return -1;
        }
         //????????????nonblock
        TcpServer::SetNonBlock(m_tickles[0]);

        stCoRoutine_t *timer_co = nullptr;
        TcpServer::task_t * ctx = (TcpServer::task_t *)calloc(1,sizeof(TcpServer::task_t));
		co_create( &timer_co,nullptr,timer_routine,ctx);

        ctx->fd = m_tickles[0];
        ctx->co = timer_co;

		co_resume( timer_co );
        return 0;
    }
}