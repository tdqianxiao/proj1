#ifndef __TADPOLE_THREAD_H__
#define __TADPOLE_THREAD_H__

#include <memory>
#include <pthread.h>
#include <functional>

#include "src/mutex.h"

namespace tadpole{

/**
 * @brief 线程类
 */
class Thread{
public:
	/**
	 * @brief 类型定义，线程类智能指针
	 */
	typedef std::shared_ptr<Thread> ptr;
	
	/**
	 * @brief 构造函数
	 * @param[in] cb 回调函数
	 * @param[in] name 线程名称
	 */
	Thread(std::function<void()> cb, const std::string & name = "unknow");

	/**
	 * @brief 析构函数
	 */
	~Thread();

	/**
	 * @brief 等待线程退出
	 */
	void join();
	
	/**
	 * @brief 获得线程id
	 */
	pid_t getThreadId()const {return m_id;}

	/**
	 * @brief 获得线程名称
	 */
	std::string getThreadName()const {return m_name;}
	
	/**
	 * @brief 获得当前线程
	 */
	static Thread *GetCurThread();
private:
	/**
	 * @brief 线程执行函数
	 */
	static void * run (void *);
private:
	//线程id
	pid_t m_id;
	//线程结构
	pthread_t m_thread;
	//信号量
	Semaphore m_sem; 
	//线程名称
	std::string m_name ; 
	//回调函数
	std::function<void()> m_cb;
};

}

#endif 

