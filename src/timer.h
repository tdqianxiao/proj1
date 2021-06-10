#ifndef __TADPOLE_TIMER_H__
#define __TADPOLE_TIMER_H__

#include <set>
#include <vector>
#include <memory>

#include "singleton.hpp"

namespace tadpole{
    class TimerManagers;
    class Timer :public std::enable_shared_from_this<Timer>{
    public:
        typedef std::shared_ptr<Timer> ptr; 

        Timer();
        Timer(uint32_t ms,std::function<void()> cb,bool isForever = false);

        int getMs();

        void reset();
        void cancel();
        void remove();

        std::function<void()> getCb()const{return m_cb;}

        bool isFor()const{return m_isForever;}
    public:
        class Compare{
        public:
            bool operator()(Timer::ptr lhs,Timer::ptr rhs){
                if( lhs && !rhs){
                    return false;
                }
                if( !lhs && rhs){
                    return true;
                }
                if( lhs->m_begin < rhs->m_begin ){
                    return true;
                }
                if( lhs->m_begin > rhs->m_begin ){
                    return false;
                } 
                return lhs.get() < rhs.get();
            }
        };
    private:
        uint32_t m_begin = 0;
        uint32_t m_end = 0;
        std::function<void()> m_cb = nullptr;
        bool m_isForever = false;
    };

    class TimerManagers{
    public:
        typedef std::shared_ptr<TimerManagers> ptr;
        typedef typename std::set<Timer::ptr,Timer::Compare> SetType;

        int frontMs();

        int delTimer(Timer::ptr timer);

        Timer::ptr addTimer(uint32_t ms,std::function<void()> cb,int isForever = false);
        Timer::ptr addTimerForever(uint32_t ms,std::function<void()> cb);

        void checkExpire();
    private:
        SetType m_timers;
    };

    typedef Singleton<TimerManagers> TimerMgr;
}

#endif 