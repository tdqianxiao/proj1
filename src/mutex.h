#ifndef __TADPOLE_MUTEX_H__
#define __TADPOLE_MUTEX_H__

#include <semaphore.h>
#include <iostream>

namespace tadpole{

/**
 * @brief 信号量类
 */
class Semaphore{
public:
	/**
	 * @brief 构造函数
	 * @param count 同时执行的任务数量
	 */
	Semaphore(const uint32_t &count = 0);

	/**
	 * @brief 析构函数
	 */
	~Semaphore();
	
	/**
	 * @brief 通知
	 */
	void notify();

	/**
	 * @brief 等待
	 */
	void wait();
private:
	//信号量结构
	sem_t m_sem; 
};

/**
 * @brief 自解锁
 */
template <class MutexType>
class SelfLock{
public:
	/**
	 * @brief 构造函数上锁
	 */
	SelfLock(MutexType & mutex)
		:m_mutex(mutex){
		if(!islock){
			m_mutex.lock();
			islock = true;
		}
	}
	
	/**
	 * @brief 上锁
	 */
	void lock(){
		if(!islock){
			m_mutex.lock();
			islock = true;
		}
	}
	
	/**
	 * @brief 解锁
	 */
	void unlock(){
		if(islock){
			m_mutex.unlock();
			islock = false;
		}
	}
	
	/**
	 * @brief 析构函数解锁
	 */
	~SelfLock(){
		unlock();
	}
private:
	//锁引用
	MutexType & m_mutex;
	//是否上锁
	bool islock = false;
};

/**
 * @brief 自解读锁
 */
template <class RDMutexType>
class RDSelfLock{
public:
	/**
	 * @brief 构造函数上锁
	 */
	RDSelfLock(RDMutexType & mutex)
		:m_mutex(mutex){
		if(!islock){
			m_mutex.rdlock();
			islock = true;
		}
	}
	
	/**
	 * @brief 上锁
	 */
	void lock(){
		if(!islock){
			m_mutex.rdlock();
			islock = true;
		}
	}
	
	/**
	 * @brief 解锁
	 */
	void unlock(){
		if(islock){
			m_mutex.unlock();
			islock = false;
		}
	}
	
	/**
	 * @brief 析构函数解锁
	 */
	~RDSelfLock(){
		unlock();
	}

private:
	//锁引用
	RDMutexType & m_mutex;
	//是否上锁
	bool islock = false;
};

/**
 * @brief 自解写锁
 */
template <class WRMutexType>
class WRSelfLock{
public:
	/**
	 * @brief 构造函数上锁
	 */
	WRSelfLock(WRMutexType & mutex)
		:m_mutex(mutex){
		if(!islock){
			m_mutex.wrlock();
			islock = true;
		}
	}

	/**
	 * @brief 上锁
	 */
	void lock(){
		if(!islock){
			m_mutex.wrlock();
			islock = true;
		}
	}


	/**
	 * @brief 解锁
	 */
	void unlock(){
		if(islock){
			m_mutex.unlock();
			islock = false;
		}
	}

	/**
	 * @brief 析构函数解锁
	 */
	~WRSelfLock(){
		unlock();
	}

private:
	//锁引用
	WRMutexType & m_mutex;
	//是否上锁
	bool islock = false;
};

/**
 * @brief 互斥锁
 */
class Mutex{
public:
	/**
	 * @brief 类型定义，自解锁的类型
	 */
	typedef SelfLock<Mutex> Lock;
public:

	/**
	 * @brief 锁初始化
	 */
	Mutex(){
		pthread_mutex_init(&m_mutex,0);
	}

	/**
	 * @brief 上锁
	 */
	void lock(){
		pthread_mutex_lock(&m_mutex);
	}

	/**
	 * @brief 解锁
	 */
	void unlock(){
		pthread_mutex_unlock(&m_mutex);
	}
	
	/**
	 * @brief 释放锁
	 */
	~Mutex(){
		pthread_mutex_destroy(&m_mutex);
	}
private:
	//互斥锁结构
	pthread_mutex_t m_mutex;
};

/**
 * @brief 读写锁
 */
class RWMutex{
public:
	/**
	 * @brief 类型定义，自解读锁的类型定义
	 */
	typedef RDSelfLock<RWMutex> RDLock;
	
	/**
	 * @brief 类型定义，自解写锁的类型定义
	 */
	typedef WRSelfLock<RWMutex> WRLock;
public:
	/**
	 * @brief 锁结构初始化
	 */
	RWMutex(){
		pthread_rwlock_init(&m_mutex,0);
	}

	/**
	 * @brief 读锁上锁
	 */
	void rdlock(){
		pthread_rwlock_rdlock(&m_mutex);
	}

	/**
	 * @brief 写锁上锁
	 */
	void wrlock(){
		pthread_rwlock_wrlock(&m_mutex);
	}
	
	/**
	 * @brief 解锁
	 */
	void unlock(){
		pthread_rwlock_unlock(&m_mutex);
	}
	
	/**
	 * @brief 锁释放
	 */
	~RWMutex(){
		pthread_rwlock_destroy(&m_mutex);
	}
private:
	//读写锁结构
	pthread_rwlock_t m_mutex;
};

/**
 * @brief 自旋锁
 */
class SpinLock{
public:
	/**
	 * @brief 自解自旋锁类型定义
	 */
	typedef SelfLock<SpinLock> Lock;
public:
	/**
	 * @brief 锁初始化
	 */
	SpinLock(){
		pthread_spin_init(&m_spin,0);
	}

	/**
	 * @brief 上锁
	 */
	void lock(){
		pthread_spin_lock(&m_spin);
	}

	/**
	 * @brief 解锁
	 */
	void unlock(){
		pthread_spin_unlock(&m_spin);
	}

	/**
	 * @brief 释放锁
	 */
	~SpinLock(){
		pthread_spin_destroy(&m_spin);
	}
private:
	//自旋锁结构
	pthread_spinlock_t m_spin;
};

}

#endif 
