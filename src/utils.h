#ifndef __TADPOLE_UTILS_H__
#define __TADPOLE_UTILS_H__

#include <string>
#include <memory>

namespace tadpole{
    extern std::string LogLevelToString(int level);
    extern std::string LuaFileRedefine(const std::string & filename);
    extern std::string GetTimeByFormat(const std::string & format = "%Y-%m-%d %H:%M:%S");
    extern uint32_t GetThreadId();
    extern uint32_t GetFiberId();
    extern uint32_t GetElapse();
    extern uint32_t GetTimeOfMS();
    extern uint32_t GetTimeOfUS();
    extern std::shared_ptr<int> GetConsumeTimePrinter();
}

#endif 