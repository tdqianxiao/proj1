#include "luaState.h"

namespace tadpole{
    luaState::luaState(){
        m_pState = luaL_newstate();
	    luaL_openlibs(m_pState); 
        reg();
    }

    void luaState::reg(){
        m_register.reset(new luaRegister(m_pState));
        m_register->reg();
    }

    int luaState::run(){
        if(m_pState){
            int ret = luaL_dofile(m_pState, "l-src/main.lua");
            if(ret != 0){
                std::cout<<lua_tostring(m_pState, -1)<<std::endl;
            }
            return ret;
        }  
        return 0;
    }

    luaState::~luaState(){
        if(m_pState){
            lua_close(m_pState);
        }  
    }
}