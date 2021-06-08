#include "src/thread.h"
#include "src/utils.h"

#include "src/log.h"

namespace tadpole{

	static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

	static thread_local Thread * t_thread = nullptr;

	Thread *Thread::GetCurThread(){
		return t_thread;
	}

	Thread::Thread(std::function<void()> cb,const std::string & name)
		:m_name(name)
		,m_cb(cb){
		int ret = pthread_create(&m_thread,nullptr,&Thread::run,this);	
		if(ret == -1){
			TADPOLE_LOG_ERROR(g_logger) << "pthread_create non success!";
			throw std::logic_error("pthread_create non success !");
		}
		m_sem.wait();
	}

	void * Thread::run(void * arg){
		t_thread = (Thread*)arg; 
		t_thread->m_id = GetThreadId();
		

		pthread_setname_np(pthread_self(), t_thread->m_name.substr(0, 15).c_str());
		t_thread->m_sem.notify();
		if(t_thread->m_cb){
			t_thread->m_cb();
		}
		return 0 ; 
	}

	Thread::~Thread(){
		if(m_thread){
			int ret = pthread_detach(m_thread);
			if(ret == -1){
				TADPOLE_LOG_ERROR(g_logger) <<"pthread_detach non success!";
			}
		}
	}

	void Thread::join(){
		if(m_thread){
			int ret = pthread_join(m_thread,nullptr);
			if(ret == -1){
				TADPOLE_LOG_ERROR(g_logger) << "pthread_join non success !";
				throw std::logic_error("pthread_join non success");
			}
			m_thread = 0; 
		}
	}
}
