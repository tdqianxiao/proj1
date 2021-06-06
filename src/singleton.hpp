#ifndef __TADPOLE_SINGLETION_HPP__
#define __TADPOLE_SINGLETION_HPP__

#include <memory>

/**
 * @brief 普通单例类
 */
template<class T>
class Singleton{
public:
	/**
	 * @brief 获得单例实例
	 */
	static T *GetInstance(){
		static T val;
		return &val;
	}
};

/**
 * @brief 智能指针单例类
 */
template<class T>
class SinletonPtr{
public:
	/**
	 * @brief 获取单例实例
	 */
	static std::shared_ptr<T> GetInstance(){
		static std::shared_ptr<T> val(new T());
		return val ;
	}
};

#endif 
