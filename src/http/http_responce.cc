#include "http_responce.h"
#include "src/log.h"
#include <sstream>

namespace tadpole{

static http_status HttpStatusFromChars(const char * status , size_t size){
#define XX(num,name,string)\
	if(strncasecmp(status,#string,size) == 0){\
		return (http_status)num ; \
	}
	HTTP_STATUS_MAP(XX);
	return http_status::HTTP_STATUS_UNKNOW;
#undef XX 
}

static http_status HttpStatusFromString(const std::string & status){
#define XX(num,name,string)\
	if(strncasecmp(status.c_str(),#string,status.size()) == 0){\
		return (http_status)num ; \
	}
	HTTP_STATUS_MAP(XX);
	return http_status::HTTP_STATUS_UNKNOW;
#undef XX 
}

static std::string HttpStatusToString(http_status status){
#define XX(num,name,string) \
	if(num == (int)status){\
		return #string ;\
	}
	HTTP_STATUS_MAP(XX);
#undef XX 
	return "UNKNOW";
}

HttpResponce::HttpResponce()
	:m_version(0x11)
	,m_status(http_status::HTTP_STATUS_OK)
	,m_close(true){
}

std::string HttpResponce::toString()const{
	std::stringstream ss; 
	ss <<"HTTP/"
       << (m_version>>4)<< "."
	   << (m_version & 0x0f)
	   << " "
	   << (int)m_status
	   << " "
	   << HttpStatusToString(m_status)
	   <<"\r\n";
	for(auto & it : m_fields){
		ss << it.first << ": " << it.second << "\r\n";
	}

	ss << "\r\n";
	ss << m_body; 
	return ss.str();
}

std::string HttpResponce::getField(const std::string & key){
	auto it = m_fields.find(key);
	if(it == m_fields.end()){
		return "";
	}
	return it->second;
}

void HttpResponce::setField(const std::string &key,const std::string & val){
	m_fields[key] = val;
}

void element_cb_status(void *data, const char *at, size_t length){
	std::cout<< std::string(at,length)<<std::endl;
	HttpResponce * req = static_cast<HttpResponce*>(data);
	http_status status = (http_status)atoi(at);	
	std::cout<< (int)status<<std::endl;
	req->setStatus(status);
}

void element_cb_version_res(void *data, const char *at, size_t length){
	HttpResponce * req = static_cast<HttpResponce*>(data);
	uint8_t version = 0 ; 
	if(strncmp(at,"HTTP/1.1",8) == 0){
		version = 0x11;
	}else if(strncmp(at, "HTTP/1.0",8) == 0){
		version = 0x10;
	}
	req->setVersion(version);
}

void field_cb_map_res(void *data, const char *field,
			  size_t flen, const char *value, size_t vlen){
	HttpResponce * req = static_cast<HttpResponce*>(data);
	req->setField(std::string(field,flen),std::string(value,vlen));
}

HttpParserResponce::HttpParserResponce()
	:m_responce(new HttpResponce){
	httpclient_parser_init(&m_parser);

	m_parser.http_field = field_cb_map_res;
	m_parser.reason_phrase = nullptr;
	m_parser.status_code = element_cb_status;
	m_parser.chunk_size = nullptr;
	m_parser.http_version = element_cb_version_res;
	m_parser.header_done = nullptr;
	m_parser.last_chunk = nullptr;

	m_parser.data = m_responce.get();
}

size_t HttpParserResponce::execute(const char * data ,size_t size){
	return httpclient_parser_execute(&m_parser,data,size,0);	
}

}
