#ifndef __TADPOLE_HTTP_HTTP_RESPONCE_H__
#define __TADPOLE_HTTP_HTTP_RESPONCE_H__

#include <memory>
#include <map>
#include <vector>
#include <string.h>

#include "httpclient_parser.h"
#include "http_macro.h"

namespace tadpole{

/**
 * @brief chars 转 httpstatus 
 */
static http_status HttpStatusFromChars(const char * status , size_t size);

/**
 * @brief string 转 httpstatus 
 */
static http_status HttpStatusFromString(const std::string & status);

/**
 * @brief httpstatus 转 string
 */
static std::string HttpStatusToString(http_status status);

/**
 * @brief http响应类
 */
class HttpResponce{
public:
	/**
	 * @brief 忽略大小写比较
	 */
	struct Compare{
		bool operator()(const std::string & lhs , const std::string & rhs){
			return strcasecmp(lhs.c_str(),rhs.c_str()) < 0;
		}
	};
public:
	/**
	 * @brief 类型定义智能指针
	 */
	typedef std::shared_ptr<HttpResponce> ptr;

	/**
	 * @brief 类型定义，field类型
	 */
	typedef std::map<std::string ,std::string , Compare> MapType;
public:	
	/**
	 * @brief 构造函数
	 */
	HttpResponce();
	
	/**
	 * @brief 转string
	 */
	std::string toString()const ;
	
	/**
	 * @brief 是否关闭连接
	 */
	bool isClose()const {return m_close;}

	/**
	 * @brief 设置是否关闭连接
	 */
	void setClose(bool close){m_close = close;}

	/**
	 * @brief 获得http版本号
	 */
	uint8_t getVersion()const {return m_version;}

	/**
	 * @brief 设置http版本
	 */
	void setVersion(uint8_t ver){m_version = ver ; }
	
	/**
	 * @brief 获得状态码
	 */
	http_status getStatus()const{return m_status;}

	/**
	 * @brief 设置状态码
	 */
	void setStatus(http_status status){m_status = status;}

	/**
	 * @brief 获得字段内容
	 * @param[in] key 键
	 */
	std::string getField(const std::string & key);

	/**
	 * @brief 设置字段内容
	 * @param[in] key 键
	 * @param[in] value 值
	 */
	void setField(const std::string & key , const std::string & val);
	
	/**
	 * @brief 获得body
	 */
	std::string getBody()const{return m_body;}

	/**
	 * @brief 设置body
	 */
	void setBody(const std::string & body){m_body = body;}

private:
	//http版本
	uint8_t m_version;
	//http响应状态码
	http_status m_status; 
	//是否关闭连接
	bool m_close ;
	//主体
	std::string m_body; 
	//字段
	MapType m_fields;
};

/**
 * @brief http响应头解析类
 */
class HttpParserResponce{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<HttpParserResponce> ptr;
	
	/**
	 * @brief 构造函数
	 */
	HttpParserResponce();

	/**
	 * @brief 开始解析响应头
	 * @param[in] data 需解析字符串地址
	 * @param[in] size 字符串长度 
	 */
	size_t execute(const char * data ,size_t size);

	/**
	 * @brief 获得已经解析的响应头
	 */
	HttpResponce::ptr getResponce()const {return m_responce;}
private:
	//解析结构
	httpclient_parser m_parser;
	//响应头结构
	HttpResponce::ptr m_responce;
};

}
#endif 
