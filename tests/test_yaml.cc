#include "src/config.h"

static tadpole::ConfigVar<std::string>::ptr g_time_format = tadpole::Config::Lookup<std::string>("time.format","%Y-%m-%d %H:%M:%S","log time format");
int main (int argv,char ** argc){
	std::cout<<g_time_format->getValue();
	return 0;	
}
