#include "luaRegister.h"
#include "utils.h"
#include "log.h"
#include <random>
#include <unistd.h>

namespace tadpole{
	//需注册的函数 start
	int assert_s(lua_State* pState)
	{
		std::string str = lua_tostring(pState, 1);
		std::cout<<str<<std::endl;
		lua_pushnumber(pState,1);
		return 1;
	}

	//打印日志
	int log(lua_State* pState){
		std::string name = lua_tostring(pState, 1);
		int level = lua_tonumber(pState, 2);
		std::string file = LuaFileRedefine(lua_tostring(pState, 3));
		int line = lua_tonumber(pState, 4);
		std::string message = lua_tostring(pState, 5);

		tadpole::Logger::ptr logger = TADPOLE_FIND_LOGGER(name);
		TADPOLE_LUA_LOG_LEVEL(LogLevel::Level(level),logger,file,line) << message;
		return 0;
	}
	//设置某位状态：setBit(val,pos,1 or 0)
	int setBit(lua_State* pState){
		int val = lua_tonumber(pState, 1);
		int pos = lua_tonumber(pState, 2);
		int bit = lua_tonumber(pState, 3);
		int set = 1 << (pos-1);
		if(bit == 1){
			val |= set; 
		}else if(bit == 0){
			val &= ~set;
		}else{
		}
		lua_pushnumber(pState,val);
		return 1;
	}

	//获得某位状态：getBit(val,pos)
	int getBit(lua_State* pState){
		int val = lua_tonumber(pState, 1);
		int pos = lua_tonumber(pState, 2);
		int set = 1 << (pos-1);
		bool ret = false;
		if(val & set){
			ret = true;
		}
		lua_pushboolean(pState,ret);
		return 1;
	}
	
	//获得一个数有多少位1
	int getBitAmount(lua_State* pState){
		int val = lua_tonumber(pState, 1);
		int amt = 0;
		while(val){
			val &= (val-1);
			++amt;
		}
		lua_pushnumber(pState,amt);
		return 1;
	}

	int rand(lua_State* pState){
		int min = lua_tonumber(pState, 1);
		int max = lua_tonumber(pState, 2);
		std::random_device rdev {}; 
		std::default_random_engine generator {rdev()}; 
		std::uniform_int_distribution<int> dist{min,max};
		int rand = dist(generator);
		lua_pushnumber(pState,rand);
		return 1;
	}

	int Sleep(lua_State* pState){
		int s = lua_tonumber(pState, 1);
		uint32_t ret = sleep(s);
		lua_pushnumber(pState,ret);
		return 1;
	}

	int SleepMs(lua_State* pState){
		int mis = lua_tonumber(pState, 1);
		uint32_t ret = usleep(mis*1000);
		lua_pushnumber(pState,ret);
		return 1;
	}


	//需注册的函数 end

	luaRegister::luaRegister(lua_State* pState)
		:m_pState(pState){
		if(m_pState){
			reg();
		}
	}
	
	//注册函数
	void luaRegister::reg(){
		lua_register(m_pState,"assert_s",assert_s);
		lua_register(m_pState,"log",log);
		lua_register(m_pState,"setBit",setBit);
		lua_register(m_pState,"getBit",getBit);
		lua_register(m_pState,"getBitAmount",getBitAmount);
		lua_register(m_pState,"rand",rand);
		lua_register(m_pState,"Sleep",Sleep);
		lua_register(m_pState,"SleepMs",SleepMs);
	}

}