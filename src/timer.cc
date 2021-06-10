#include "timer.h"
#include "utils.h"

namespace tadpole{
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
}