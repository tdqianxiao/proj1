#include "src/config.h"
#include "src/log.h"

namespace tadpole{

void Config::InitFromYaml(const YAML::Node & node,const std::string &prefix, std::map<std::string , std::string> & configItems){
	if(node.IsMap()){
		for(auto it = node.begin(); it != node.end(); ++it){
			std::string slice = prefix.empty() ? "" : ".";
			std::string str = prefix + slice + it->first.Scalar();
			std::stringstream ss ; 
			ss << it->second;
			configItems.insert(std::make_pair(str,ss.str()));
			InitFromYaml(it->second,str,configItems);
		}	
	}else if(node.IsSequence()){
		for(auto it = node.begin(); it != node.end(); ++it){
			InitFromYaml(*it,prefix,configItems);
		}
	}else if (node.IsScalar()){
		std::stringstream ss;
		ss << node;
		configItems.insert(std::make_pair(prefix,ss.str()));
	}
}

void Config::LookupData(std::map<std::string , std::string> & configItems){
	RWMutex::RDLock lock(GetMutex());
	auto &data = GetData();
	lock.unlock();
	for(auto &it : configItems){
		lock.lock();
		auto iter = data.find(it.first);
		lock.unlock();
		RWMutex::WRLock lock2(GetMutex());
		if(iter != data.end()){
			iter->second->fromString(it.second);	
		}
	}
}

bool Config::LoadFromYaml(const std::string & filename){
	try{
		YAML::Node node = YAML::LoadFile(filename);
	    std::map<std::string , std::string> configItems;
		InitFromYaml(node,"",configItems);
		LookupData(configItems);	
		return true; 
	}catch(...){
		return false ; 
	}
}

}
