#include "src/log.h"
#include "src/config.h"

namespace tadpole{

//%d 输出日志时间点的日期或时间
//%F 输出日志信息所在的文件
//%l 输出日志事件的发生位置，即输出日志信息的语句处于它所在的类的第几行
//%t 输出产生该日志事件的线程名
//%f 输出日志所在的协程
//%p 输出优先级，即DEBUG，INFO，WARN，ERROR，FATAL。如果是调用debug()输出的，则为DEBUG，依此类推
//%c 输出日志信息所属的类的全名
//%m 输出代码中指定的信息，如log(message)中的message
//%n 输出一个回车换行符，Windows平台为“rn”，Unix平台为“n”
//%e 输出当前程序运行开始到现在的毫秒数
//%% 直接输出%

class DateFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<DateFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level, 
						LogEvent::ptr event)override {
		char mytime[64] = {0};
		time_t tv = event->getTime();
		strftime(mytime,sizeof(mytime),"%Y-%m-%d %H:%M:%S",localtime(&tv));
		os << mytime;
	}
};

class FileFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<FileFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getFilename();
	}
};

class LineFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<LineFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getLine();
	}
};

class ThreadFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<ThreadFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getThreadId();
	}
};


class FiberFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<FiberFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getFiberId();
	}
};

class LevelFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<LevelFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << LogLevel::ToString(level);
	}
};

class NameFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<NameFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getLoggerName();
	}
};

class MessageFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<MessageFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getContent();
	}
};

class NewLineFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<NewLineFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << std::endl;
	}
};

class TableFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<TableFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << "\t";
	}
};

class StringFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<StringFormatItem> ptr;
	
	StringFormatItem(const std::string & str)
		:m_str(str){
	}
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << m_str;
	}

private:
	std::string  m_str;
};

class ElapseFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<ElapseFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os <<event->getElapse();
	}
};

class PercentFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<PercentFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << "%";
	}
};

//end formatItem 

EventWrap::EventWrap(Logger::ptr logger ,LogLevel::Level level,
					 LogEvent::ptr event)
	:m_logger(logger)
	,m_event(event)
	,m_level(level){
}

std::stringstream & EventWrap::getSS(){
	return m_event->getSS();
}

EventWrap::~EventWrap(){
	m_logger->log(m_level,m_event);
}

std::string LogLevel::ToString(LogLevel::Level level){
	switch(level){
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARN:
		return "WARN";
	case LogLevel::ERROR:
		return "ERROR";
	case LogLevel::FATAL:
		return "FATAL";
	default : 
		return "UNKNOW";
	}
}

LogLevel::Level LogLevel::FromString(std::string val){
	std::transform(val.begin(),val.end(),val.begin(),::tolower);
#define XX(name)\
	if(val == #name){\
		return LogLevel::name;\
	}

	XX(DEBUG)
	XX(INFO)
	XX(WARN)
	XX(ERROR)
	XX(FATAL)
#undef XX
	return LogLevel::UNKNOW;
}

LogEvent::LogEvent(uint32_t line,uint32_t threadid,
			 uint32_t fiberid, uint64_t time,
			 uint64_t elapse,const std::string & filename
			 ):m_line(line)
			  ,m_threadId(threadid)
			  ,m_fiberId(fiberid)
			  ,m_time(time)
			  ,m_elapse(elapse)
			  ,m_filename(filename){}

Logger::Logger(const std::string & name)
	:m_name(name){
}

void Logger::log(LogLevel::Level level , LogEvent::ptr event){
	event->setLoggerName(m_name);
	if(level >= m_level){
		for(auto & it : m_logAppenders){
			if(level >= it->getLevel()){
				it->format(level,event);
			}
		}
	}
}

void Logger::addAppender(LogAppender::ptr append){
	if(append){
		m_logAppenders.push_back(append);
	}
}

void Logger::delAppender(LogAppender::ptr append){
	for(auto it = m_logAppenders.begin(); it != m_logAppenders.end();++it){
		if(*it == append){
			m_logAppenders.erase(it);
			return ; 
		}
	}
}

void Logger::clearAppender(){
	m_logAppenders.clear();
}

void Logger::debug(LogEvent::ptr event){
	log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event){
	log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event){
	log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event){
	log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event){
	log(LogLevel::FATAL, event);
}

void LogAppender::setFormat(LogFormatter::ptr format){
	m_logFormatter= format;
}

void LogAppender::setFormat(const std::string & format){
	m_logFormatter.reset(new LogFormatter(format));
}

LogAppender::LogAppender(){
	m_logFormatter.reset(new LogFormatter());
}

StdoutLogAppender::StdoutLogAppender(){
}

StdoutLogAppender::~StdoutLogAppender(){
}

void StdoutLogAppender::format(LogLevel::Level level,LogEvent::ptr event){
	if(m_logFormatter){
		std::cout<<m_logFormatter->format(level,event);
	}
}

FileLogAppender::FileLogAppender(const std::string & name)
	:m_filename(name){
	reopen();
	m_type = 1;
}
bool FileLogAppender::reopen(){
	if(m_fileStream){
		m_fileStream.close();
	}
	
	m_fileStream.open(m_filename,std::ios::trunc);

	return !!m_fileStream;
}
FileLogAppender::~FileLogAppender(){
	m_fileStream.close();
}

void FileLogAppender::format(LogLevel::Level level, LogEvent::ptr event) {
	if(m_logFormatter){
		m_fileStream << m_logFormatter->format(level,event);
	}
}

std::string LogFormatter::format(LogLevel::Level level, LogEvent::ptr event){
	std::stringstream ss; 
	ss.clear();
	for(auto & it : m_formatItems){
		it->format(ss,level,event);	
	}
	return ss.str();
}

void LogFormatter::init(){
	//first type 0-string , 1 format , second format 
	std::vector<std::pair<int , std::string>> vec;
	
	std::string str;
	std::string fstr; 
	int f = 0 ;
	int n = 0 ; 
	for(int i = 0 ; i < (int)m_pattern.size() ; ++i){
		if(m_pattern[i] != '%'){
			continue; 
		}
		n = i+1;
		str = m_pattern.substr(f,i-f);
		if(n < (int)m_pattern.size()){
				fstr = m_pattern.substr(n,1);
				f = n + 1 ; 
				i = n ;
		}
		if(!str.empty()){
			vec.push_back(std::make_pair(0,str));
			str.clear();
		}
		vec.push_back(std::make_pair(1,fstr));
		fstr.clear();
	}
	str = m_pattern.substr(f,m_pattern.size() - f);
	vec.push_back(std::make_pair(0,str));

	static std::map<std::string , std::function<FormatItem::ptr()> > s_fmt_map_str = {
#define XX(fmt,item) \
	std::make_pair(\
		fmt,[]()->FormatItem::ptr{\
			return item::ptr(new item());\
		}\
	),
	
	XX("d",DateFormatItem)
	XX("F",FileFormatItem)
	XX("l",LineFormatItem)
	XX("t",ThreadFormatItem)
	XX("f",FiberFormatItem)
	XX("p",LevelFormatItem)
	XX("c",NameFormatItem)
	XX("m",MessageFormatItem)
	XX("n",NewLineFormatItem)
	XX("T",TableFormatItem)
	XX("e",ElapseFormatItem)
	XX("%",PercentFormatItem)
#undef XX
	};

	for(auto & it : vec){
		if(it.first == 0){
			m_formatItems.push_back(StringFormatItem::ptr(new StringFormatItem(it.second)));
		}else{
			auto item = s_fmt_map_str.find(it.second);
			if(item != s_fmt_map_str.end()){
				m_formatItems.push_back(item->second());
			}else {
				m_formatItems.push_back(StringFormatItem::ptr(new StringFormatItem("<error>")));
			}
		}
	}
}

LogFormatter::LogFormatter(const std::string& pattern)
	:m_pattern(pattern){
	if(m_pattern.empty()){
		m_pattern = "%d%T%e%T[%c]%T[%p]%T[%F:%l]%T[%t:%f]%T%m%n";//默认输出格式
	}
	init();
}

Logger::ptr LoggerMgr::getLogger(const std::string & name){
	auto it = m_loggers.find(name);
	if(it == m_loggers.end()){
		auto logger = Logger::ptr(new Logger(name));
		m_loggers.insert(make_pair(name,logger));
		return logger;
	}
	return it->second;
}

//type=0 Stdout type=1 file
struct LogAppDef{
	LogAppDef()
	:type(0)
	,level(LogLevel::UNKNOW)
	,filename("")
	,format(""){
	}
	int type ; 
	LogLevel::Level level;
	std::string filename; 
	std::string format;
};

struct LogDef{
	LogDef()
	:name("")
	,level(LogLevel::UNKNOW){
	}
	std::string name ; 
	LogLevel::Level level; 
	std::vector<LogAppDef> appends;
	
	bool operator<(const LogDef & rhs){
		if(name < rhs.name){
			return true; 
		}else if(name > rhs.name) {
			return false; 
		}

		if(level < rhs.level){
			return true; 
		}else if (level > rhs.level){
			return false; 
		}

		return appends.size() < appends.size();
	}
};

//日志结构体和字符串转换，偏特化
template <>
class LexicalCast<std::string,LogDef>{
public:
	LogDef operator()(const std::string & str){
		LogDef log;
		YAML::Node node = YAML::Load(str);
		if(node["name"].IsDefined()){
			log.name = node["name"].as<std::string>();
		}
		if(node["level"].IsDefined()){
			std::string level = node["level"].as<std::string>();
			log.level = LogLevel::FromString(level);
		}else{
			log.level = LogLevel::UNKNOW;
		}
		if(node["appends"].IsDefined()){
			YAML::Node apps = node["appends"];
			for(auto it = apps.begin(); it != apps.end();
				++it){
				LogAppDef apd;
				YAML::Node fp;
				if((*it)["StdoutLogAppender"].IsDefined()){
					apd.type = 0 ;
					fp = (*it)["StdoutLogAppender"];
				}else if((*it)["FileLogAppender"].IsDefined()){
					apd.type = 1 ; 
					fp = (*it)["FileLogAppender"];
					if(fp["path"].IsDefined()){
						apd.filename = fp["path"].as<std::string>();
					}	
				}

				if(fp["level"].IsDefined()){
					std::string level = fp["level"].as<std::string>();
					apd.level = LogLevel::FromString(level);
				}else{
					apd.level = LogLevel::UNKNOW;
				}
				if (fp["format"].IsDefined()){
					apd.format = fp["format"].as<std::string>();
				}
			
				log.appends.push_back(apd);
			}
		}
		return log;
	}
};

template <>
class LexicalCast<LogDef,std::string>{
public:
	std::string operator()(const LogDef& log){
		YAML::Node node; 
		node["name"] = log.name;
		node["level"] = LogLevel::ToString(log.level);
		
		YAML::Node apds;
		for(auto & it : log.appends){
			YAML::Node fp ; 
			if(it.type == 0){
				fp["level"] = LogLevel::ToString(it.level);	
				fp["format"] = it.format;
				apds["StdoutLogAppender"] = fp;
			}else if(it.type == 1){	
				fp["level"] = LogLevel::ToString(it.level);
				fp["format"] = it.format;
				fp["path"] = it.filename;
				apds["FileLogAppender"] = fp;
			}
		}
		node["appends"].push_back(apds);

		std::stringstream ss; 
		ss << node;
		return ss.str();
	}
};

void LoggerMgr::fromYaml(const LogDef & log){
	if(log.name.empty()){
		return ;
	}
	Logger::ptr logger = getLogger(log.name);
	logger->setLevel(log.level);
	for(auto &it : log.appends){
		LogAppender::ptr apd ; 
		if(it.type == 0){
			apd.reset(new StdoutLogAppender);
		}else if (it.type == 1){
			if(!it.filename.empty()){
				apd.reset(new FileLogAppender(it.filename));
			}
		}
		apd->setLevel(it.level);
		if(!it.format.empty()){
			apd->setFormat(it.format);
		}
		logger->addAppender(apd);
	}
}

std::string LoggerMgr::toYamlString(){
	std::vector<LogDef> vec;
	vec.resize(m_loggers.size());
	int i = 0 ; 
	for(auto &logger : m_loggers){
		vec[i].level = logger.second->getLevel();
		vec[i].name = logger.second->getName();
		for(auto &it : logger.second->getAppends()){
			LogAppDef lad ;
			lad.level = it->getLevel();
			lad.format = it->getFormatString();
			if(it->getType() == 1){
				lad.filename = std::dynamic_pointer_cast<FileLogAppender>(it)->getFileName();
				lad.type = 1;
			}else {
				lad.type = 0 ; 
			}
			vec[i].appends.push_back(lad);
		}
	}
	try{
		return LexicalCast<std::vector<LogDef>,std::string>()(vec);
	}catch(...){
		return "";
	}
}

struct LogIniter{
	LogIniter(){
		ConfigVar<std::vector<LogDef> >::ptr g_logger_def = Config::Lookup<std::vector<LogDef> >("logs",{},"config logs");
		Config::LoadFromYaml("./config/yaml/config.yml");
		for(auto & it : g_logger_def->getValue()){
			LogMgr::GetInstance()->fromYaml(it);
		}
	}
};

static LogIniter __log_init;
}

