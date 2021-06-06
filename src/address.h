#ifndef __TADPOLE_ADDRESS_H__
#define __TADPOLE_ADDRESS_H__

#include <memory>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <vector>
#include <map>

namespace tadpole{

/**
 *@brief 地址类
 */
class Address{
public:
	
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<Address> ptr;
	
	/**
	 * @brief 域名转地址
	 * @param[out] ips 保存地址的vector
	 * @param[in] ipordns 需要转换的ip或dns
	 * @param[in] service 服务类型，比如http
	 */
	static void Lookup(std::vector<Address::ptr> & ips,const char * ipordns, 
										const char * service = 0);

	/**
	 * @brief 获得接口信息
	 * @param[out] ips 用于保存本机上所有网卡ip地址
	 */
	static void LookupInterfaces(
			std::multimap<std::string,Address::ptr> & ips);

	/**
	 * @brief 虚析构
	 */
	virtual ~Address(){}
	
	/**
	 * @brief 获得地址族协议
	 */
	int getFamily();
	
	/**
	 * @brief 获得该地址的结构
	 */
	virtual const sockaddr* getAddr()const = 0 ;

	/**
	 * @brief 可改的获得地址
	 */
	virtual sockaddr* getAddr() = 0 ;

	/**
	 * @brief 获得该地址结构的长度
	 */
	virtual socklen_t getAddrLen()const = 0 ;
	
	/**
	 * @brief 插入地址的明文到流
	 * @param[out] os 输出流
	 * @return 输出流引用
	 */
	virtual std::ostream & insert(std::ostream & os) = 0;
	std::string toString();
	
	/**
	  * @brief 比较操作符，为了让其再set或其它容器中比较
	  */
	bool operator< (const Address & rhs)const;
	bool operator== (const Address & rhs)const;
	bool operator!= (const Address &rhs)const; 
};

/**
 * @brief ip地址
 */
class IPAddress:public Address{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<IPAddress> ptr;

	/**
	 * @brief 根据地址结构创建不同的ip地址
	 */
	static IPAddress::ptr CreateByAddr(const sockaddr * addr , const socklen_t& len);
	/**
	 * @brief 根据ip地址创建ip地址
	 * @param[in] ip 地址
	 * @param[in] port 端口号
	 */
	static IPAddress::ptr Create(const char * ip , uint16_t port);
	
	/**
	 * @brief 获得广播地址
	 */
	virtual IPAddress::ptr getBroadcast(uint32_t prefix) = 0 ;
	
	/**
	 * @brief 获得网络地址
	 */
	virtual IPAddress::ptr getNetwork(uint32_t prefix) = 0;

	/**
	 * @brief 获得子网掩码
	 */
	virtual IPAddress::ptr getSubnetmask(uint32_t prefix) = 0 ; 
	
	/**
	 * @brief 获得端口号
	 */
	virtual uint32_t getPort()const = 0 ;

	/**
	 * @brief 设置端口号
	 */
	virtual void setPort(uint16_t port) = 0;
};

/**
 * @brief ipv4地址
 */
class IPv4Address:public IPAddress{
public:
	/**
	 * @brief ipv4类智能指针定义
	 */
	typedef std::shared_ptr<IPv4Address> ptr;
	
	/** 
	 * @brief 构造函数
	 * @param[in] ip ip地址
	 * @param[in] port 端口
	 */
	IPv4Address(uint32_t ip = INADDR_ANY, uint16_t port = 0 );
	
	/**
	 * @brief 构造函数
	 * @param[in] addr ipv4地址结构
	 * @param[in] len ipv4地址结构长度
	 */
	IPv4Address(const sockaddr * addr,const socklen_t& len);
	/**
	 * @brief 创建一个ipv4地址
	 */
	static IPv4Address::ptr Create(const char * ip , uint16_t port);

	/** 
	 * @breif 以下实现父类的虚函数
	 */
	const sockaddr* getAddr()const override ;
	sockaddr* getAddr()override;
	socklen_t getAddrLen()const override ;
	std::ostream & insert(std::ostream & os)override;
	IPAddress::ptr getBroadcast(uint32_t prefix) override ;
	IPAddress::ptr getNetwork(uint32_t prefix) override;
	IPAddress::ptr getSubnetmask(uint32_t prefix) override; 
	uint32_t getPort()const override;
	void setPort(uint16_t port) override;

private: 
	//ipv4地址结构
	sockaddr_in m_addr;
};

/**
 * @brief ipv6地址类
 */
class IPv6Address:public IPAddress{
public:
	/**
	 * @brief 定义ipv6类智能指针
	 */
	typedef std::shared_ptr<IPv6Address> ptr;
	
	/**
	 * @brief 无参构造，创建后地址，端口为空
	 */
	IPv6Address();

	/**
	 * @brief 构造函数
	 * @param[in] addr ipv6地址结构
	 * @param[in] len ipv6地址结构长度
	 */
	IPv6Address(const sockaddr * addr,const socklen_t & len);

	/**
	 * @brief 构造函数
	 * @param[in] ip ipv6地址，类型为uint8数组
	 * @param[in] port 端口
	 */
	IPv6Address(const uint8_t * ip , uint16_t port = 0 );
	
	/**
	 * @brief 创建一个ipv6地址
	 * @param[in] ip ipv6地址，类型为字符串
	 * @param[in] port 端口
	 */
	static IPv6Address::ptr Create(const char * ip , uint16_t port);

	/**
	 * @brief 实现父类的虚函数
	 */
	const sockaddr* getAddr()const override ;
	sockaddr* getAddr()override ;
	socklen_t getAddrLen()const override ;
	std::ostream & insert(std::ostream & os)override;
	IPAddress::ptr getBroadcast(uint32_t prefix) override ;
	IPAddress::ptr getNetwork(uint32_t prefix) override;
	IPAddress::ptr getSubnetmask(uint32_t prefix) override; 
	uint32_t getPort()const override;
	void setPort(uint16_t port) override;
private:
	//ipv6地址结构
	sockaddr_in6 m_addr;
};

/**
 * @brief 本地套接字地址
 */
class UnixAddress:public Address{
public:
	/**
	 * @ brief 智能指针定义
	 */
	typedef std::shared_ptr<UnixAddress> ptr;
	
	/**
	 * @brief 构造函数
	 */
	UnixAddress();

	/**
	 * @brief 构造函数
	 * @param[in] path 用于套接字地址的文件路径
	 */
	UnixAddress(const std::string & path);

	/**
	 * @brief 设置长度
	 */
	void setLen(int len){m_length = len;}

	/**
	 * @brief 实现父类的虚函数
	 */
	const sockaddr* getAddr()const override ;
	sockaddr* getAddr()override ;
	socklen_t getAddrLen()const override ;
	std::ostream & insert(std::ostream & os)override;
private:
	//本地套接字地址结构
	sockaddr_un m_addr;
	//地址结构长度
	socklen_t m_length;
};

/**
 * @brief 未知地址
 */
class UnKnowAddress:public Address{
public:
	/** 
	 * @brief 定义智能指针
	 */
	typedef std::shared_ptr<UnKnowAddress> ptr;
	
	/**
	 * @brief 构造函数
	 */
	UnKnowAddress();

	/**
	 * @brief 实现父类的虚函数 
	 */
	const sockaddr* getAddr()const override ;
	sockaddr* getAddr()override ; 
	socklen_t getAddrLen()const override ;
	std::ostream & insert(std::ostream & os)override;
private:
	//地址结构
	sockaddr m_addr;
};

}

#endif 
