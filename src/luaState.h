#ifndef __TADPOLE_LUA_STATE_H__
#define __TADPOLE_LUA_STATE_H__

#include "luaRegister.h"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace tadpole{
    class luaState{
    public:
        typedef std::shared_ptr<luaState> ptr;
    public:
        luaState();
        ~luaState();

        void reg();
        int run();
        lua_State* getState()const{return m_pState;}
    private:
        lua_State* m_pState = nullptr;
        luaRegister::ptr m_register = nullptr; 
    };
}

#endif 