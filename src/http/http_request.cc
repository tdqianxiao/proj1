#include "http_request.h"

namespace tadpole{

std::string HttpMethodToString(http_method method){
#define XX(num,name,string) \
	if(num == (int)method){\
		return #name ;\
	}
	HTTP_METHOD_MAP(XX);
#undef XX 
	return "UNKNOW";
}


http_method HttpMethodFromString(const std::string & method){
#define XX(num,name,string)\
	if(strncmp(method.c_str(),#name,method.size())  == 0){\
		return (http_method)num ; \
	}
	HTTP_METHOD_MAP(XX);
	return http_method::HTTP_UNKNOW;
#undef XX 
}

http_method HttpMethodFromChars(const char * method,size_t length){
#define XX(num,name,string)\
	if(strncmp(method,#name,length)  == 0){\
		return (http_method)num; \
	}
	HTTP_METHOD_MAP(XX);
	return http_method::HTTP_UNKNOW;
#undef XX 
}

HttpRequest::HttpRequest()
	:m_method(http_method::HTTP_GET)
	,m_close(true)
	,m_version(0x11)
	,m_path("/"){
	
}

std::string HttpRequest::getField(const std::string & key){
	auto it = m_fields.find(key);	
	if(it == m_fields.end()){
		return "";
	}
	return it->second;
}

void HttpRequest::setField(const std::string & key , const std::string & val){
	m_fields[key] = val ; 
}

std::string HttpRequest::toString()const{
	std::stringstream ss; 
	ss << HttpMethodToString(m_method)<<" "<< m_path
	   <<(m_query.empty() ? "" :"?" )
	   << m_query
	   <<(m_flagment.empty() ? "" : "#")
	   << m_flagment
	   << " HTTP/"
	   <<(m_version>>4)<< "."<<(m_version & 0x0f)
	   << "\r\n";

	for(auto & it : m_fields){
		ss << it.first << ": " << it.second << "\r\n";
	}

	ss << "\r\n";
	return ss.str();
}


void element_cb_method(void *data, const char *at, size_t length){
	HttpRequest * req = static_cast<HttpRequest*>(data);
	http_method method = HttpMethodFromChars(at,length);	
	req->setMethod(method);
}

void element_cb_version(void *data, const char *at, size_t length){
	HttpRequest * req = static_cast<HttpRequest*>(data);
	uint8_t version = 0 ; 
	if(strncmp(at,"HTTP/1.1",8) == 0){
		version = 0x11;
	}else if(strncmp(at, "HTTP/1.0",8) == 0){
		version = 0x10;
	}
	req->setVersion(version);
}

void element_cb_path(void *data, const char *at, size_t length){
	HttpRequest * req = static_cast<HttpRequest*>(data);
	req->setPath(std::string(at,length));
}

void element_cb_fragment(void *data, const char *at, size_t length){
	HttpRequest * req = static_cast<HttpRequest*>(data);
	req->setFlagment(std::string(at,length));
}

void element_cb_query(void *data, const char *at, size_t length){
	HttpRequest * req = static_cast<HttpRequest*>(data);
	req->setQuery(std::string(at,length));
}

void field_cb_map(void *data, const char *field,
			  size_t flen, const char *value, size_t vlen){
	HttpRequest * req = static_cast<HttpRequest*>(data);
	req->setField(std::string(field,flen),std::string(value,vlen));
}

HttpRequestParser::HttpRequestParser()
	:m_request(new HttpRequest){
	http_parser_init(&m_parser);

	m_parser.http_field = field_cb_map;
	m_parser.request_method = element_cb_method;
	m_parser.request_uri = nullptr;
	m_parser.fragment = element_cb_fragment;
	m_parser.request_path = element_cb_path;
	m_parser.query_string = element_cb_query;
	m_parser.http_version = element_cb_version;
	m_parser.header_done = nullptr;

	m_parser.data = m_request.get();
}

size_t HttpRequestParser::execute(char * data,size_t len){
	size_t off = http_parser_execute(&m_parser, data, len ,0 );
	memmove(data, data + off, (len - off));
	return off;
}

}
