#ifndef __TADPOLE_HTTP_HTTP_REQUEST_H__
#define __TADPOLE_HTTP_HTTP_REQUEST_H__

#include <memory>
#include <map>
#include <vector>
#include <string.h>
#include <sstream>
#include "http_macro.h"
#include "http11_parser.h"

namespace tadpole{

/**
 * @brief 将http方法转换为字符串
 * @param[in] mothod 方法HTTP_?
 */
static std::string HttpMethodToString(http_method method);

/**
 * @brief 将字符串转为方法
 */
static http_method HttpMethodFromString(const std::string & method);

/**
 * @brief 将字符串转换为方法，字符串为c风格字符串
 */
static http_method HttpMethodFromChars(const char * method);

/**
 * @brief http请求类
 */
class HttpRequest{
public:
	/** 
	 * @brief 仿函数，用来忽略map比较时的大小写
	 */
	struct Compare{
		bool operator()(const std::string & lhs , const std::string & rhs){
			return strcasecmp(lhs.c_str(),rhs.c_str()) < 0;
		}
	};
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<HttpRequest> ptr;

	/**
	 * @brief 类型定义，字段键值对map类型
	 */
	typedef std::map<std::string,std::string,Compare> MapType; 
public:
	/**
	 * @brief 构造函数
	 */
	HttpRequest();
	
	/**
	 * @brief 获得方法
	 */
	http_method getMethod()const {return m_method;}

	/**
	 * @brief 设置方法
	 */
	void setMethod(http_method method){m_method = method;}

	/**
	 * @brief 是否关闭连接
	 */
	bool isClose()const {return m_close;}

	/**
	 * @brief 设置是否关闭连接
	 */
	void setClose(bool close){m_close = close;}

	/**
	 * @brief 获得http版本
	 */
	uint8_t getVersion()const {return m_version;}

	/**
	 * @brief 设置http版本
	 */
	void setVersion(uint8_t ver){m_version = ver ; }

	/**
	 * @brief 获得字段
	 * @param[in] key 根据键找字段
	 * @return 返回字段，没有返回""
	 */
	std::string getField(const std::string & key);

	/** 
	 * @brief 设置字段
	 * @param[in] key 键
	 * @param[in] val 值
	 */
	void setField(const std::string & key , const std::string & val);

	/**
	 * @brief 设置请求的路径
	 */
	void setPath(const std::string & path){m_path = path ;}

	/**
	 * @brief 获得请求的路径
	 */
	std::string getPath()const {return m_path;}

	/**
	 * @brief 设置uri？后面的那个东西
	 */
	void setFlagment(const std::string & flag){m_flagment = flag;}

	/**
	 * @brief 获得uri? 后面的那个东西
	 */
	std::string getFlgment()const{return m_flagment;}

	/**
	 * @brief 获得主体
	 */
	std::string getBody()const{return m_body;}

	/**
	 * @brief 设置主体内容
	 */
	void setBody(const std::string & body){m_body = body;}
	
	/**
	 * @brief 获得uri#后面的东西
	 */
	std::string getQuery()const {return m_query;}

	/**
	 * @brief 设置uri#后面的东西
	 */
	void setQuery(const std::string & query){m_query = query;}
	
	/**
	 * @brief 转换为字符串
	 */
	std::string toString()const ;
private:
	//请求的方法
	http_method m_method ; 
	//是否关闭连接
	bool m_close; 
	//http版本
	uint8_t m_version;
	//请求路径
	std::string m_path ; 
	//?
	std::string m_flagment;
	//#
	std::string m_query; 
	//头部字段值
	MapType m_fields;
	//主体内容
	std::string m_body; 
	//cookies
	std::vector<std::string> m_cookies;
};

/**
 * @brief http请求解析类
 */
class HttpRequestParser{
public: 
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<HttpRequestParser> ptr; 
	
	/**
	 * @brief 构造函数
	 */
	HttpRequestParser();

	/**
	 * @brief 开始解析
	 */
	size_t execute(char * data,size_t len);

	/**
	 * @brief 是否解析完成
	 */
	bool isFinish(){return http_parser_is_finished(&m_parser);}
	
	/**
	 * @brief 获得解析完的请求类
	 */
	HttpRequest::ptr getRequest(){return m_request;}
private:
	//解析回调的结构
	http_parser m_parser;
	//请求
	HttpRequest::ptr  m_request;
};

}

#endif 
