#include "src/thread.h"
#include <unistd.h>
#include "src/log.h"

static tadpole::Logger::ptr logger = TADPOLE_FIND_LOGGER("root");

void func1(){
    while(1){
        TADPOLE_LOG_INFO(logger)<<"haha";
    }
}

void func2(){
    while(1){
        TADPOLE_LOG_INFO(logger)<<"heihei";
    }
}


int main (int argv,char ** argc){
    for(int i = 0 ; i < 3 ; ++i){
        tadpole::Thread::ptr th1 (new tadpole::Thread(func1,"xxx"));
        std::cout<<th1->getThreadId()<<"------"<<th1->getThreadName()<<std::endl;
        tadpole::Thread::ptr th2 (new tadpole::Thread(func2,"yyy"));
        std::cout<<th2->getThreadId()<<"------"<<th2->getThreadName()<<std::endl;
    }

    while(1);
	return 0;	
}
