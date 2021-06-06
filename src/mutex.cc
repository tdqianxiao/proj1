#include "src/mutex.h"
#include "src/log.h"

namespace tadpole{

	static Logger::ptr logger = TADPOLE_FIND_LOGGER("system");

	Semaphore::Semaphore(const uint32_t &count){
		int ret = sem_init(&m_sem,0,count);
		if(ret == -1){
			TADPOLE_LOG_ERROR(logger)<<"sem_init non success !";
			throw std::logic_error("sem_init non success !");
		}
	}

	Semaphore::~Semaphore(){
		int ret = sem_destroy(&m_sem);
		if(ret == -1){
			TADPOLE_LOG_ERROR(logger)<<"sem_destroy no success !";
		}
	}

	void Semaphore::notify(){
		int ret = sem_post(&m_sem);
		if(ret == -1){
			TADPOLE_LOG_ERROR(logger)<<"sem_post non success !";
			throw std::logic_error("sem_post non success !");
		}
	}

	void Semaphore::wait(){
		int ret = sem_wait(&m_sem);
		if(ret == -1){
			TADPOLE_LOG_ERROR(logger)<<"sem_wait non success !";
			throw std::logic_error("sem_wait non success !");
		}
	}
}
