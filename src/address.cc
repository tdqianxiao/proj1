#include "address.h"
#include "bswap.hpp"
#include "log.h"
#include "utils.h"
#include <ifaddrs.h>
#include <netdb.h>

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

namespace tadpole{

void Address::Lookup(std::vector<Address::ptr> & ips,
					 const char * ipordns, const char * service){
	addrinfo hints,*result = nullptr;
	hints.ai_family = AF_UNSPEC;
	memset(&hints,0,sizeof(hints));

	int ret = getaddrinfo(ipordns,service,&hints,&result);
	if(ret){
		TADPOLE_LOG_ERROR(g_logger) << "Address::look("<< ipordns << ":"
									<< service << ")ret = "<<ret ;
		return;
	}
	
	addrinfo *temp = result; 
	if(result){
		for(;;){
			Address::ptr ip ; 
			if(temp->ai_family == AF_INET){
				ip.reset(new IPv4Address(temp->ai_addr
								,temp->ai_addrlen));
			}else if(temp->ai_family == AF_INET6){
				ip.reset(new IPv6Address(temp->ai_addr
								,temp->ai_addrlen));	
			}else{
				ip.reset(new UnKnowAddress());
			}
			ips.push_back(ip);
			if(temp->ai_next){
				temp = temp->ai_next;
			}else {
				break;
			}
		}
		freeaddrinfo(result);
	}
}

void Address::LookupInterfaces(std::multimap<std::string,Address::ptr> &ips){
	struct ifaddrs * temp , * result;
	if(-1 == getifaddrs(&result)){
		TADPOLE_LOG_ERROR(g_logger)<< "LookupInterfaces() error ! errno: "
								   << errno << " strerr: "
								   << strerror(errno);
		return ; 
	}
	
	temp = result;
	if(result){
		for(;;){
			Address::ptr ip ; 
			if(temp->ifa_addr->sa_family == AF_INET){
				ip.reset(new IPv4Address(temp->ifa_addr
								,sizeof(temp->ifa_addr)));
			}else if(temp->ifa_addr->sa_family == AF_INET6){
				ip.reset(new IPv6Address(temp->ifa_addr
								,sizeof(temp->ifa_addr)));	
			}
			if(ip){
				ips.insert(std::make_pair(temp->ifa_name,ip));
			}
			if(temp->ifa_next){
				temp = temp->ifa_next;
			}else {
				break;
			}
		}
		freeifaddrs(result);
	}
}

int Address::getFamily(){
	return getAddr()->sa_family;
}
	
std::string Address::toString(){
	std::stringstream ss; 
	insert(ss);
	return ss.str();
}

inline sockaddr *IPv4Address::getAddr(){
	return (sockaddr*)&m_addr;
}

inline sockaddr *IPv6Address::getAddr(){
	return (sockaddr*)&m_addr;
}

inline sockaddr *UnixAddress::getAddr(){
	return (sockaddr*)&m_addr;
}

inline sockaddr *UnKnowAddress::getAddr(){
	return (sockaddr*)&m_addr;
}

bool Address::operator< (const Address & rhs)const{
	socklen_t min_len = std::min(getAddrLen(),rhs.getAddrLen());
	int result = memcmp(getAddr(),rhs.getAddr(),min_len);

	if(result < 0 ){
		return true;
	}else if(result > 0){
		return false;
	}else if(getAddrLen() < rhs.getAddrLen()){
		return true;
	}
	return true;
}

inline bool Address::operator== (const Address & rhs)const{
	return(getAddrLen() == rhs.getAddrLen() &&
	   	   memcmp(getAddr(),rhs.getAddr(),getAddrLen()));
}

inline bool Address::operator!= (const Address &rhs)const{
	return !(*this == rhs);
}


IPAddress::ptr IPAddress::CreateByAddr(const sockaddr * addr , const socklen_t& len){
	if(addr->sa_family == AF_INET){
		return IPv4Address::ptr(new IPv4Address(addr,len));
	}else if(addr->sa_family == AF_INET6){
		return IPv6Address::ptr(new IPv6Address(addr,len));
	}
	return nullptr;
}

IPAddress::ptr IPAddress::Create(const char * ip , uint16_t port){
	addrinfo hints , *result = nullptr ; 
	memset(&hints,0,sizeof(hints));
	hints.ai_flags = AI_NUMERICHOST;
	hints.ai_family = AF_UNSPEC;
	int ret = getaddrinfo(ip,nullptr,&hints, &result);

	if(ret){
		TADPOLE_LOG_ERROR(g_logger) << "IPAddress::Create("<< ip << ":"
									<< port << ")ret = "<<ret ;
		return nullptr;
	}
	IPAddress::ptr ipv =  CreateByAddr(result->ai_addr,result->ai_addrlen);
	ipv->setPort(port);
	if(result){
		freeaddrinfo(result);
	}
	return ipv;
}

IPv4Address::IPv4Address(uint32_t ip, uint16_t port){
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = byteswapOnLittleEndian(port);
	m_addr.sin_addr.s_addr = byteswapOnLittleEndian(ip);
}

IPv4Address::IPv4Address(const sockaddr * addr, const socklen_t & len){
	memcpy(&m_addr,addr,len);
}

IPv4Address::ptr IPv4Address::Create(const char * ip,uint16_t port){
	IPv4Address::ptr ipv4(new IPv4Address());
	int ret = inet_pton(AF_INET,ip,&ipv4->m_addr.sin_addr.s_addr);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"create ipv4 error ! ip : "<<ip
								<<" port: "<<port<<" errno = "
								<<errno;
		return nullptr;
	}
	ipv4->m_addr.sin_port = byteswapOnLittleEndian(port);
	return ipv4;
}

inline const sockaddr* IPv4Address::getAddr()const  {
	return (sockaddr*)&m_addr;
}

inline socklen_t IPv4Address::getAddrLen()const  {
	return sizeof(m_addr);
}

std::ostream & IPv4Address::insert(std::ostream & os){
	uint32_t ip = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
	os<<"["<<(ip>>24)<< "."
			<<((ip>>16) & 0xff)<< "."
			<<((ip>>8) & 0xff)<< "."
			<<(ip & 0xff)<<"]:"
			<<byteswapOnLittleEndian(m_addr.sin_port);
	return os;
}

IPAddress::ptr IPv4Address::getBroadcast(uint32_t prefix)  {
	if(prefix > 32)return nullptr;
	uint32_t ip = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
	ip |= (1<<(32-prefix))-1;
	return IPv4Address::ptr(new IPv4Address(ip,byteswapOnLittleEndian(m_addr.sin_port)));
}

IPAddress::ptr IPv4Address::getNetwork(uint32_t prefix) {
	if(prefix > 32)return nullptr;
	uint32_t ip = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
	ip &= ~((1<<(32-prefix))-1);
	return IPv4Address::ptr(new IPv4Address(ip,byteswapOnLittleEndian(m_addr.sin_port)));
}

IPAddress::ptr IPv4Address::getSubnetmask(uint32_t prefix) {
	if(prefix > 32)return nullptr;
	int ip = ~((1<<(32-prefix)) - 1);
	return IPv4Address::ptr(new IPv4Address(ip,byteswapOnLittleEndian(m_addr.sin_port)));
} 

inline uint32_t IPv4Address::getPort()const {
	return byteswapOnLittleEndian(m_addr.sin_port);
}

inline void IPv4Address::setPort(uint16_t port) {
	m_addr.sin_port = byteswapOnLittleEndian(port);
}

IPv6Address::IPv6Address(const sockaddr * addr,const socklen_t & len){
	memcpy(&m_addr,addr,len);
}

IPv6Address::IPv6Address(){
	memset(&m_addr,0,sizeof(m_addr));
}

IPv6Address::IPv6Address(const uint8_t * ip , uint16_t port ){	
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin6_family = AF_INET6;
	m_addr.sin6_port = byteswapOnLittleEndian(port);
	memcpy(m_addr.sin6_addr.s6_addr,ip,16);
}

IPv6Address::ptr IPv6Address::Create(const char * ip , uint16_t port){
	IPv6Address::ptr ipv6(new IPv6Address);
	int ret = inet_pton(AF_INET6,ip,&ipv6->m_addr.sin6_addr);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger) << "ipv6 Create no success !";
		return nullptr;
	}
	ipv6->m_addr.sin6_port = byteswapOnLittleEndian(port);
	ipv6->m_addr.sin6_family = AF_INET6;
	return ipv6;
}

inline const sockaddr* IPv6Address::getAddr()const  {
	return (sockaddr*)&m_addr;
}

inline socklen_t IPv6Address::getAddrLen()const  {
	return sizeof(m_addr);
}

std::ostream & IPv6Address::insert(std::ostream & os){
	os <<"[";
	char buf[INET6_ADDRSTRLEN] = {0};
	const char * str = inet_ntop(AF_INET6,&m_addr.sin6_addr.s6_addr,buf,sizeof(buf));
	os<<str;
	os << "]:"<<byteswapOnLittleEndian(m_addr.sin6_port);
	return os;
}

IPAddress::ptr IPv6Address::getBroadcast(uint32_t prefix)  {	
	if(prefix > 128)return nullptr;
	sockaddr_in6 sock6(m_addr);
	uint16_t * tempipv6 = (uint16_t *)sock6.sin6_addr.s6_addr;
	int mid = prefix/16;
	tempipv6[mid] = tempipv6[mid] | (uint16_t)((1<<(prefix%16))-1);
	for(int i = mid + 1 ; i < 8 ; ++i){
		tempipv6[i] = 0xffff;
	}
	return IPv6Address::ptr(new IPv6Address((sockaddr*)&sock6,sizeof(sock6)));
}

IPAddress::ptr IPv6Address::getNetwork(uint32_t prefix) {
	if(prefix > 128)return nullptr;
	sockaddr_in6 sock6(m_addr);
	uint16_t * tempipv6 =(uint16_t*)sock6.sin6_addr.s6_addr;
	int mid = prefix/16;
	tempipv6[mid] = tempipv6[mid] & ~(uint16_t)((1<<(prefix%16))-1);
	for(int i = mid + 1 ; i < 8 ; ++i){
		tempipv6[i] = 0;
	}
	return IPv6Address::ptr(new IPv6Address((sockaddr*)&sock6,sizeof(sock6)));
}

IPAddress::ptr IPv6Address::getSubnetmask(uint32_t prefix) {
	sockaddr_in6 sock6 = {0};
	sock6.sin6_port = m_addr.sin6_port;
	uint16_t * tempipv6 = (uint16_t*)sock6.sin6_addr.s6_addr;
	int mid = prefix/16;
	tempipv6[mid] = ~(uint16_t)((1<<(prefix%16))-1);
	for(int i = 0 ; i < mid ; ++i){
		tempipv6[i] = 0xffff;
	}
	return IPv6Address::ptr(new IPv6Address((sockaddr*)&sock6,sizeof(sock6)));
} 

inline uint32_t IPv6Address::getPort()const {
	return byteswapOnLittleEndian(m_addr.sin6_port);
}

inline void IPv6Address::setPort(uint16_t port) {
	m_addr.sin6_port = byteswapOnLittleEndian(port);
}

static const size_t MAX_PATH = sizeof(sockaddr_un) - offsetof(sockaddr_un,sun_path) -1;

UnixAddress::UnixAddress(){
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sun_family = AF_UNIX;
	m_length = offsetof(sockaddr_un,sun_path) + MAX_PATH;
}

UnixAddress::UnixAddress(const std::string & path){	
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sun_family = AF_UNIX;
	m_length = path.size() + 1;

	if(path.empty()){
		m_length = 0 ; 
	}
	if(path.size() >MAX_PATH){
		throw std::logic_error("unixAddress path is long !");
	}

	memcpy(m_addr.sun_path,path.c_str(),m_length);
	m_length += offsetof(sockaddr_un,sun_path);
}

inline const sockaddr* UnixAddress::getAddr()const  {
	return (sockaddr*)&m_addr;
}

inline socklen_t UnixAddress::getAddrLen()const  {
	return m_length;
}

std::ostream & UnixAddress::insert(std::ostream & os){
	os<<"[file:"<<m_addr.sun_path<<"]";
	return os;
}

UnKnowAddress::UnKnowAddress(){
}

inline const sockaddr* UnKnowAddress::getAddr()const  {
	return &m_addr;
}

inline socklen_t UnKnowAddress::getAddrLen()const  {
	return sizeof(m_addr);
}

std::ostream & UnKnowAddress::insert(std::ostream & os){
	os<<"[UnKnowAddress]";
	return os; 
}
}
