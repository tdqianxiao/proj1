#ifndef __TADPOLE_LUA_REGISTER_H__
#define __TADPOLE_LUA_REGISTER_H__

#include <iostream>
#include <memory>
#include <vector>
#include <string>

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace tadpole{
    class luaRegister{
    public:
        typedef std::shared_ptr<luaRegister> ptr;
    public:
        luaRegister(lua_State* pState);
        void reg();
    private:
        lua_State* m_pState = nullptr;
    };
}

#endif 