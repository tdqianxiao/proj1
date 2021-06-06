#ifndef __TADPOLE_LOG_H__
#define __TADPOLE_LOG_H__

#include <memory>
#include <vector>
#include <map>
#include <tuple>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <algorithm>

#include "src/singleton.hpp"
#include "src/utils.h"
#include "src/mutex.h"
/**
 * @根据日志级别打印不同级别的日志
 */
#define TADPOLE_LOG_LEVEL(level,logger) \
	tadpole::EventWrap::ptr(new tadpole::EventWrap(logger,level,tadpole::LogEvent::ptr(new tadpole::LogEvent(__LINE__\
										 ,tadpole::GetThreadId(),tadpole::GetFiberId()\
										 ,time(NULL),tadpole::GetElapse(),__FILE__))))->getSS()

/**
 * @根据日志级别打印不同级别的日志,lua 
 */
#define TADPOLE_LUA_LOG_LEVEL(level,logger,file,line) \
	tadpole::EventWrap::ptr(new tadpole::EventWrap(logger,level,tadpole::LogEvent::ptr(new tadpole::LogEvent(line\
										 ,tadpole::GetThreadId(),tadpole::GetFiberId()\
										 ,time(NULL),tadpole::GetElapse(),file))))->getSS()

/**
 * @brief 根据名字获得日志器
 */
#define TADPOLE_FIND_LOGGER(logger) \
	tadpole::LogMgr::GetInstance()->getLogger(logger)

/**
 * @brief 分别输出不同级别的日志，用logger输出；
 */
#define TADPOLE_LOG_DEBUG(logger) TADPOLE_LOG_LEVEL(tadpole::LogLevel::DEBUG,logger)

#define TADPOLE_LOG_INFO(logger) TADPOLE_LOG_LEVEL(tadpole::LogLevel::INFO,logger)

#define TADPOLE_LOG_WARN(logger) TADPOLE_LOG_LEVEL(tadpole::LogLevel::WARN,logger)

#define TADPOLE_LOG_ERROR(logger) TADPOLE_LOG_LEVEL(tadpole::LogLevel::ERROR,logger)

#define TADPOLE_LOG_FATAL(logger) TADPOLE_LOG_LEVEL(tadpole::LogLevel::FATAL,logger)

namespace tadpole{

///声明
class Logger;

/**
 * @brief 日志事件类
 */
class LogEvent{
public:
	/**
	 * @brief 类型定义，日志事件智能指针
	 */
	typedef std::shared_ptr<LogEvent> ptr;
	
	/**
	 * @brief 构造函数
	 * @param[in] line 所在行号
	 * @param[in] threadid 线程id
	 * @param[in] fiberid 协程id
	 * @param[in] filename 所在文件名
	 */
	LogEvent(uint32_t line,uint32_t threadid,
			 uint32_t fiberid, uint64_t time,
			 uint64_t elapse,const std::string & filename
			 );
	/**
	 * @brief 获得行号
	 */
	uint32_t getLine()const {return m_line;}

	/**
	 * @brief 获得线程id
	 */
	uint32_t getThreadId()const {return m_threadId;}

	/**
	 * @brief 获得协程id
	 */
	uint32_t getFiberId()const {return m_fiberId;}

	/**
	 * @brief 获得系统时间
	 */
	uint64_t getTime()const {return m_time;}

	/**
	 * @brief 获得时间戳
	 */
	uint64_t getElapse()const {return m_elapse;}

	/**
	 * @brief 获得所在文件名
	 */
	std::string getFilename()const {return m_filename;}

	/**
	 * @brief 获得打印日志的日志器名称
	 */
	std::string getLoggerName()const {return m_loggerName;}

	/**
	 * @brief 获得日志输出的实际内容
	 */
	std::string getContent()const {return m_ss.str();}

	/**
	 * @brief 设置日志名称
	 * @param[in] name 日志器名称
	 */
	void setLoggerName(const std::string & name){m_loggerName = name;}
	
	/**
	 * @brief 获得当前之间的字符流引用，用于输入日志实际内容
	 */
	std::stringstream& getSS(){return m_ss;}
private:
	//所在行号
	uint32_t m_line = 0 ; 

	//所在线程id,为ps里可见的哪个id
	uint32_t m_threadId = 0 ;

	//所在协程id
	uint32_t m_fiberId = 0 ;

	//当前时间
	uint64_t m_time = 0 ;

	//时间戳，毫秒级
	uint64_t m_elapse = 0 ; 
	
	//所在文件
	std::string m_filename; 

	//日志器名称
	std::string m_loggerName; 

	//日志内容字符流
	std::stringstream m_ss;
};

/**
 * @brief 日志级别类
 */
class LogLevel{
public:
	/**
	 * @brief 6个日志级别，越大越严重
	 */
	enum Level{
		UNKNOW = 0 ,
		DEBUG = 1,
		INFO = 2 ,
		WARN = 3,
		ERROR = 4,
		FATAL = 5
	};
	
	/**
	 * @brief 将日志级别转换为字符串
	 * @param[in] level 日志级别
	 */
	static std::string ToString(LogLevel::Level level);

	/**
	 * @brief 从字符串转换为日志级别
	 * @param[in] val 日志级别明文字符串
	 */
	static LogLevel::Level FromString(std::string val);
};

/**
 * @brief 事件封装类
 */
class EventWrap{
public:
	/**
	 * @brief 类型定义，事件封装类的智能指针
	 */
	typedef std::shared_ptr<EventWrap> ptr;

	/**
	 * @brief 构造函数
	 * @param[in] logger 日志器
	 * @param[in] level 日志级别
	 * @param[in] event 日志事件
	 */
	EventWrap(std::shared_ptr<Logger> logger ,LogLevel::Level level, LogEvent::ptr event); 

	/**
	 * @brief 获得事件字符流引用，用于输入日志实际内容
	 */
	std::stringstream & getSS();

	/**
	 * @brief 析构函数，用日志器打印日志
	 */
	~EventWrap();
private:
	//日志器
	std::shared_ptr<Logger> m_logger;
	//日志事件
	LogEvent::ptr m_event; 
	//日志级别
	LogLevel::Level m_level;
};

/**
 * @brief 日志格式类
 */
class LogFormatter{
public:
	/**
	 * @brief 类型定义，日志格式类的智能指针
	 */
	typedef std::shared_ptr<LogFormatter> ptr;
	
	/**
	 * @brief 构造函数
	 * @brief param[in] pattern 日志打印格式的字符串
	 */
	LogFormatter(const std::string& pattern = "");

	/**
	 * @brief 格式化打印字符串
	 * @param[in] level 日志级别
	 * @param[in] event 日志事件
	 */
	std::string format(LogLevel::Level level, LogEvent::ptr event);

	/**
	 * @brief 日志格式子项类，内部类
	 */
	class FormatItem{
	public:
		/**
	     * @brief 类型定义，日志格式子项类的智能指针
	     */
		typedef std::shared_ptr<FormatItem> ptr;
		
		/**
		 * @brief 虚析构，供子类继承
		 */
		virtual ~FormatItem(){}

		/**
		 * @brief 格式化各种格式到os
		 * @param[out] os 输出流引用
		 * @param[in] level 日志级别
		 * @param[in] event 日志事件
		 */
		virtual void format(std::ostream & os , LogLevel::Level level ,
							LogEvent::ptr event) = 0;
	};
	
	/**
	 * @brief 判断是否有格式
	 * @return true表示已有格式
	 */
	bool hasFormat(){return !m_pattern.empty();}

	/**
	 * @brief 解析pattern格式
	 */
	void init();
	
	/**
	 * @brief 获得原始需解析的字符串
	 */
	const std::string& getPattern()const {return m_pattern;}
private:
	//日志格式字符串
	std::string m_pattern ; 
	//子项vec
	std::vector<FormatItem::ptr> m_formatItems;
};

/**
 * @ 日志输出地
 */
class LogAppender{
public:
	/**
	 * @brief 类型定义,日志输出地的智能指针
	 */
	typedef std::shared_ptr<LogAppender> ptr;
	
	/**
	 * @brief 构造函数
	 */
	LogAppender();

	/**
	 * @brief 虚析构，供继承
     */
	virtual ~LogAppender(){}
	
	/**
	 * @brief 纯虚函数，格式化输出日志
	 * @param level 日志级别
	 * @param event 日志事件
	 */
	virtual void format(LogLevel::Level level,LogEvent::ptr event) = 0;

	/**
	 * @brief 获取日志级别
	 */
	LogLevel::Level getLevel()const {return m_level;}

	/**
	 * @brief 设置日志级别
	 */
	void setLevel(LogLevel::Level level){m_level = level;}

	/**
	 * @brief 设置输出格式
	 * @param[in] format 日志格式对象
	 */
	void setFormat(LogFormatter::ptr format);

	/**
	 * @brief 设置输出格式
	 * @param[in] format 日志格式字符串
	 */
	void setFormat(const std::string & format);

	/**
	 * @brief 获得日志格式字符串
	 */
	std::string getFormatString()const {return m_logFormatter->getPattern();}

	/**
	 * @brief 获得日志类型
	 */
	int getType()const{return m_type;}
protected:
	//日志格式
	LogFormatter::ptr m_logFormatter;
	//日志级别，默认为UNKNOW
	LogLevel::Level m_level = LogLevel::UNKNOW; 
	//日志类型
	int m_type = 0 ; 
};

/**
 * @brief 控制台日志，继承日志输出地类
 */
class StdoutLogAppender:public LogAppender{
public:
	/**
	 * @brief 类型定义，控制台日志的智能指针
	 */
	typedef std::shared_ptr<StdoutLogAppender> ptr;
	
	/**
	 * @brief 构造函数
	 */
	StdoutLogAppender();

	/**
	 * @brief 析构函数
	 */
	~StdoutLogAppender();

	/**
	 * @brief 重写format
	 * @param[in] level 日志级别
	 * @param[in] event 日志事件
	 */
	void format(LogLevel::Level level,LogEvent::ptr event) override ;
};

/**
 * @brief 文件输出地类，继承自日志输出地类
 */
class FileLogAppender:public LogAppender{
public:
	/**
	 * @brief 类型定义，文件输出地类的智能指针
	 */
	typedef std::shared_ptr<FileLogAppender> ptr ;
	
	/**
	 * @brief 构造函数
	 * @param name 文件名
	 */
	FileLogAppender(const std::string & name);
	
	/**
	 * @brief 析构函数
	 */
	~FileLogAppender();

	/**
	 * @brief 重写format
	 * @param[in] level 日志级别
	 * @param[in] event 日志事件
	 */
	void format(LogLevel::Level level, LogEvent::ptr event) override ;
	
	/**
	 * @brief 重新打开文件
	 */
	bool reopen();

	/**
	 * @brief 获得文件名
	 */
	const std::string &getFileName()const {return m_filename;}
private:
	//文件流
	std::ofstream m_fileStream;
	//文件名
	std::string m_filename ; 
};

/**
 * @brief 日志器类
 */
class Logger{
public:
	/**
	 * @brief 类型定义，日志器类的智能指针
	 */
	typedef std::shared_ptr<Logger> ptr;

	/** 
	 * @brief 互斥锁
	 */
	typedef Mutex MutexType;

	/** 
	 * @brief 读写锁
	 */
	typedef RWMutex MutexType2;
public:
	/**
	 * @brief 构造函数
	 * @param[in] name 日志器名称，默认root
	 */
	Logger(const std::string & name = "root");

	/**
	 * @brief 打印日志
	 * @param[in] level 日志级别
	 * @param[in] event 日志事件
	 */
	void log(LogLevel::Level level , LogEvent::ptr event);

	/**
	 * @brief 打印debug级别的日志
	 * @param[in] event 日志事件
	 */
	void debug(LogEvent::ptr event);
	
	/**
	 * @brief 打印info级别的日志
	 * @param[in] event 日志事件
	 */
	void info(LogEvent::ptr event);
	
	/**
	 * @brief 打印warn级别的日志
	 * @param[in] event 日志事件
	 */
	void warn(LogEvent::ptr event);
	
	/**
	 * @brief 打印error级别的日志
	 * @param[in] event 日志事件
	 */
	void error(LogEvent::ptr event);
	
	/**
	 * @brief 打印fatal级别的日志
	 * @param[in] event 日志事件
	 */
	void fatal(LogEvent::ptr event);

	/**
	 *@brief 设定日志器日志级别
	 */
	void setLevel(LogLevel::Level level){m_level = level;}

	/**
	 * @brief 添加输出地
	 * @param[in] append 输出地
	 */
	void addAppender(LogAppender::ptr append);

	/**
	 * @brief 清除所有输出地
	 */
	void clearAppender();
	
	/**
	 * @brief 删除输出地
	 * @param[in] append 输出地
	 */
	void delAppender(LogAppender::ptr append);

	/**
	 * @brief 获得该日志所有的输出地
	 */
	std::vector<LogAppender::ptr> getAppends()const {return m_logAppenders;}

	/**
	 * @brief 获得日志器的日志级别
	 */
	LogLevel::Level getLevel()const {return m_level;}

	/**
	 * @brief 获得日志器名称
	 */
	std::string getName()const {return m_name;}

private:
	//日志输出地
	std::vector<LogAppender::ptr> m_logAppenders ;

	//日志名称
	std::string m_name ; 

	//日志级别
	LogLevel::Level m_level = LogLevel::UNKNOW;

	//互斥锁
	MutexType m_lock; 

	//读写锁
	MutexType2 m_rwlock;
};

struct LogDef;
/**
 * @brief 日志器管理类
 */
class LoggerMgr{
public:
	typedef RWMutex MutexType;
public:
	/**
	 * @brief 获得日志器,没有会自动创建
	 * @param[in] name 日志器名称
	 */
	Logger::ptr getLogger(const std::string & name);
	
	/**
	 * @brief 从yaml读到的结构提转换为日志器
	 */
	void fromYaml(const LogDef &);
	
	/**
	 * @brief 将日志器转换为yaml字符串
	 */
	std::string toYamlString();

private:
	/**
	 * @brief 日志器map 
	 */
	std::map<std::string,Logger::ptr> m_loggers ; 

	///读写锁
	MutexType m_lock;
};

/**
 * @brief 类型定义，将日志器管理类设置为单例类
 */
typedef Singleton<LoggerMgr> LogMgr;

}

#endif
