#ifndef __TADPOLE_CONFIG_H__
#define __TADPOLE_CONFIG_H__

#include <memory>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

#include "yaml.h"

namespace tadpole{

/**
 * @brief 配置基类
 */
class ConfigVarBase{
public:
	
	/**
	 * @brief 类型定义，配置基类的智能指针
	 */
	typedef std::shared_ptr<ConfigVarBase> ptr;
	
	/**
	 * @brief 构造函数
	 * @param[in] name 配置名称
	 * @param[in] description 配置描述
	 */
	ConfigVarBase(const std::string & name , const std::string & description)
		:m_name(name)
		,m_description(description){
	}
	
	/**
	 *  @brief 虚析构函数，供继承
	 */
	virtual ~ConfigVarBase(){}
	
	/**
	 * @brief 获得配置名称
	 */
	std::string getName()const {return m_name;}

	/**
	 * @brief 获得描述信息
	 */
	std::string getDescription()const {return m_description;}
	
	/**
	 * @brief 纯虚函数，将配置转换为yaml字符串
	 */
	virtual std::string toString() = 0 ; 

	/**
	 * @brief 纯虚函数,根据yaml字符串更改自己的值
	 */
	virtual void fromString(const std::string & str) = 0 ; 
protected:
	//配置名称
	std::string m_name ;
	//描述信息
	std::string m_description; 
};

/**
 * @brief 类型转换，用的是boost::lexical_cast
 */
template <class From, class To>
class LexicalCast{
public:
	To operator()(const From & val){
		return boost::lexical_cast<To>(val);
	}
};

/**
 * @brief 字符串转字符串，为了同步yaml字符串
 */
template <>
class LexicalCast<std::string,std::string>{
public:
	std::string operator()(const std::string & str){
		if(str.empty()){
			return str;
		}
		if(str[0] == '\"' && str[str.size()-1] == '\"'){
			std::string string(str.begin()+1,str.end()-1);
			return string;
		}
		return str;
	}
};

/**
 * @brief vector转字符串
 */
template <class T>
class LexicalCast<std::vector<T>,std::string>{
public:
	std::string operator()(std::vector<T> val){
		YAML::Node node; 
		std::stringstream ss; 
		for(size_t i = 0 ; i < val.size(); ++i){
			node[i] = LexicalCast<T,std::string>()(val[i]);
		}
		ss << node;
		return ss.str();
	}
};

/**
 * @brief 字符串转vector
 */
template <class T>
class LexicalCast<std::string,std::vector<T> >{
public:
	std::vector<T> operator()(const std::string & str){
		YAML::Node node = YAML::Load(str);
		std::vector<T> vec;
		for(auto it = node.begin(); it != node.end() ; ++it){
			try{
				std::stringstream ss;
				ss << *it; 
				vec.push_back(LexicalCast<std::string,T>()(ss.str()));
			}catch(...){
				return vec;
			}
		}
		return vec;
	}
};

/**
 * @brief list转字符串
 */
template <class T>
class LexicalCast<std::list<T>,std::string>{
public:
	std::string operator()(std::list<T> val){
		YAML::Node node; 
		std::stringstream ss; 
		int i = 0 ;
		for(auto it = val.begin(); it != val.end(); ++it,++i){
			node[i] = LexicalCast<T,std::string>()(*it);
		}
		ss << node;
		return ss.str();
	}
};

/**
 * @brief 字符串转list
 */
template <class T>
class LexicalCast<std::string,std::list<T> >{
public:
	std::list<T> operator()(const std::string & str){
		YAML::Node node = YAML::Load(str);
		std::list<T> lis;
		for(auto it = node.begin(); it != node.end() ; ++it){
			try{
				std::stringstream ss; 
				ss << *it ; 
				lis.push_back(LexicalCast<std::string,T>()(ss.str()));
			}catch(...){
				return lis;
			}
		}
		return lis;
	}
};

/**
 * @brief set转字符串
 */
template <class T>
class LexicalCast<std::set<T>,std::string>{
public:
	std::string operator()(std::set<T> val){
		YAML::Node node; 
		std::stringstream ss; 
		int i = 0 ; 
		for(auto it = val.begin(); it != val.end(); ++it,++i){
			node[i] = LexicalCast<T,std::string>()(*it);
		}
		ss << node;
		return ss.str();
	}
};

/**
 * @brief 字符串转set
 */
template <class T>
class LexicalCast<std::string,std::set<T> >{
public:
	std::set<T> operator()(const std::string & str){
		YAML::Node node = YAML::Load(str);
		std::set<T> se;
		for(auto it = node.begin(); it != node.end() ; ++it){
			try{
				std::stringstream ss; 
				ss << *it ; 
				se.insert(LexicalCast<std::string,T>()(ss.str()));
			}catch(...){
				return se;
			}
		}
		return se;
	}
};

/**
 * @brief unordered_set转字符串
 */
template <class T>
class LexicalCast<std::unordered_set<T>,std::string>{
public:
	std::string operator()(std::unordered_set<T> val){
		YAML::Node node; 
		std::stringstream ss; 
		int i = 0 ; 
		for(auto it = val.begin(); it != val.end(); ++it,++i){
			node[i] = LexicalCast<T,std::string>()(*it);
		}
		ss << node;
		return ss.str();
	}
};

/**
 * @brief 字符串转unordered_set
 */
template <class T>
class LexicalCast<std::string,std::unordered_set<T> >{
public:
	std::unordered_set<T> operator()(const std::string & str){
		YAML::Node node = YAML::Load(str);
		std::unordered_set<T> se;
		for(auto it = node.begin(); it != node.end() ; ++it){
			try{
				std::stringstream ss; 
				ss << *it ; 
				se.insert(LexicalCast<std::string,T>()(ss.str()));
			}catch(...){
				return se;
			}
		}
		return se;
	}
};

/**
 * @brief map转字符串
 */
template <class T>
class LexicalCast<std::map<std::string,T>,std::string>{
public:
	std::string operator()(std::map<std::string,T> val){
		YAML::Node node; 
		std::stringstream ss; 
		int i = 0 ; 
		for(auto it = val.begin(); it != val.end(); ++it,++i){
			node[it->first] = LexicalCast<T,std::string>()(it->second);
		}
		ss << node;
		return ss.str();
	}
};

/**
 * @brief 字符串转map
 */
template <class T>
class LexicalCast<std::string,std::map<std::string,T> >{
public:
	std::map<std::string,T> operator()(const std::string & str){
		YAML::Node node = YAML::Load(str);
		std::map<std::string,T> se;
		for(auto it = node.begin(); it != node.end() ; ++it){
			try{
				std::stringstream ss; 
				ss << it->second ; 
				se.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));
			}catch(...){
				return se;
			}
		}
		return se;
	}
};

/**
 * @brief unordered_map转字符串
 */
template <class T>
class LexicalCast<std::unordered_map<std::string,T>,std::string>{
public:
	std::string operator()(std::unordered_map<std::string,T> val){
		YAML::Node node; 
		std::stringstream ss; 
		int i = 0 ; 
		for(auto it = val.begin(); it != val.end(); ++it,++i){
			node[it->first] = LexicalCast<T,std::string>()(it->second);
		}
		ss << node;
		return ss.str();
	}
};

/**
 * @brief 字符串转unordered_map
 */
template <class T>
class LexicalCast<std::string,std::unordered_map<std::string,T> >{
public:
	std::unordered_map<std::string,T> operator()(const std::string & str){
		YAML::Node node = YAML::Load(str);
		std::unordered_map<std::string,T> se;
		for(auto it = node.begin(); it != node.end() ; ++it){
			try{
				std::stringstream ss; 
				ss << it->second ; 
				se.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));
			}catch(...){
				return se;
			}
		}
		return se;
	}
};

/**
 * @brief 配置项类
 */
template <class T>
class ConfigVar: public ConfigVarBase{
public:
	
	/**
	* @brief 类型定义，ConfigVar的智能指针
 	*/
	typedef std::shared_ptr<ConfigVar> ptr;
	
	/**
	 * @brief 构造函数
	 * @param[in] name 配置名称
	 * @param[in] val 配置具体值
	 * @param[in] description 配置值描述
	 */
	ConfigVar(const std::string & name , const T & val , const std::string & description)
		:ConfigVarBase(name,description)
		,m_val(val){}
		
	/**
	 * @brief 析构函数 
	 */
	~ConfigVar(){}
		
	/**
	 * @brief 重写父类的toString
	 */
	std::string toString()override{
		try{
			return LexicalCast<T,std::string>()(m_val);
		}catch(...){
			std::cout<< "lexical_cast error : "<<
							typeid(m_val).name()<<"convent to string non success!";
							return "";
		}
	}
	
	
	/**
	 * @brief 重写父类的fromString
	 */
	void fromString(const std::string & str)override{
		try{
			T temp_val = LexicalCast<std::string,T>()(str);
			if(m_cb){
				m_cb(m_val,temp_val);
			}
			m_val = temp_val;
		}catch(...){
			std::cout<< "lexical_cast error : string convent to "<<
						typeid(m_val).name()<<" non success!";
		}
	}
		
	/**
	 * @brief 为配置项添加监听事件，若更改配置则会调用
	 * @param[in] old_val 老的配置值
	 * @param[in] new_val 新的配置值
	 */
	void addListener(std::function<void(const T& old_val , const T& new_val)> cb){
		m_cb = cb ; 
	}

	/**
	 * @brief 获得具体配置值
	 */
	T getValue()const {return m_val ; }
private:
	//具体配置值
	T m_val ; 
	//监听回调函数
	std::function<void(const T& old_val , const T& new_val)> m_cb;
};

/**
 * @brief 配置类，用来管理配置项的
 */
class Config{
public:
	/**
	 * @brief 查看是否有这项配置，有则更改， 无则产生
	 * @param[in] name 配置名称
	 * @param[in] val 配置值
	 * @param[in] description 描述
	 * @return 配置项智能指针
	 */
	template <class T>
	static typename ConfigVar<T>::ptr Lookup(const std::string & name 
		, const T & val , const std::string & description = ""){
		auto &data = GetData();
		auto it = data.find(name);
		typename ConfigVar<T>::ptr conf(new ConfigVar<T>(name , val , description));
		if(it == data.end()){
			data.insert(std::make_pair(name,conf));
		}else{
			it->second = conf;
		}
		return conf;
	}
	
	/**
	 * @brief 加载配置文件
	 * @paramp[in] filename 文件名
	 */
	static bool LoadFromYaml(const std::string & filename);
private:
	/**
	 * @brief 初始化加载的yaml配置
	 * @param[in] node yaml节点
	 * @param[in] p 配置前缀
	 * @param[out] configItems 保存解析的配置项
	 */
	static void InitFromYaml(const YAML::Node & node,const std::string & p,std::map<std::string,std::string> & configItems);
	
	/**
	 * @brief 根据配置看是否需要更改原配置
	 * @param[in] 解析之后的配置项
	 */
	static void LookupData(std::map<std::string,std::string> & configItems);
	
	/**
	 * @具体配置值map,配置项存放位置
	 */
	static std::map<std::string,ConfigVarBase::ptr>& GetData(){
		static std::map<std::string,ConfigVarBase::ptr> s_data;
		return s_data;
	}
};

}

#endif 
