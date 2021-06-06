#include <string.h>
#include <time.h>
#include <chrono>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "utils.h"
#include "log.h"
using namespace std::chrono;

#define gettid() syscall(__NR_gettid)

#include <iostream>

namespace tadpole{
    static time_point<high_resolution_clock> g_begin_time = high_resolution_clock::now();
    static Logger::ptr logger = TADPOLE_FIND_LOGGER("system");

    std::string LogLevelToString(int level){
        switch (level)
        {
        case 1:
            return "DEBUG";
        case 2:
            return "INFO";
        case 3:
            return "WARN";
        case 4:
            return "ERROR";
        case 5:
            return "FATAL";
        default:
            return "UNKNOW";
        }
    }

    std::string LuaFileRedefine(const std::string & filename){
        int offset = strlen("l-src/");
        int len = filename.length();
        int pos = filename.find("l-src/");
        std::string res = filename.substr(pos+offset,len-pos-offset);
        return std::move(res);
    }   

    std::string GetTimeByFormat(const std::string & format){
        char buf[64] = {};
        time_t ltime;
        time(&ltime);
        struct tm *localTime;
        localTime = localtime(&ltime);
        strftime(buf,64,format.c_str(),localTime);
        return std::string(buf);
    }

    uint32_t GetThreadId(){
        return gettid(); 
    }

    uint32_t GetFiberId(){
        return 0; 
    }
    
    uint32_t GetElapse(){
        return duration_cast<microseconds>(high_resolution_clock::now() - g_begin_time).count() / 1000;
    }

    uint32_t GetTimeOfMS(){
        struct timeval tm= {0};
        gettimeofday(&tm,nullptr);
        return tm.tv_sec*1000 + tm.tv_usec/1000;
    }

    uint32_t GetTimeOfUS(){
        struct timeval tm= {0};
        gettimeofday(&tm,nullptr);
        return tm.tv_sec*1000*1000 + tm.tv_usec;
    }

    std::shared_ptr<int> GetConsumeTimePrinter(){
        time_point<high_resolution_clock> begin = high_resolution_clock::now();
        std::shared_ptr<int> printer(new int,[begin](void * ptr){
            uint32_t tm = duration_cast<microseconds>(high_resolution_clock::now() - begin).count() / 1000;
            TADPOLE_LOG_INFO(logger) << "use time : "<< tm << " ms !";
        });
        return printer;
    }

}