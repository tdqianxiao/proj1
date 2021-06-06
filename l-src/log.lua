class("LOG_TYPE")

LOG_TYPE.DEBUG = 1
LOG_TYPE.INFO = 2 
LOG_TYPE.WARN = 3 
LOG_TYPE.ERROR = 4 
LOG_TYPE.FATAL = 5 

function c_log(name,level,...)
    local info = {...}
    local message = ""
    for k,v in pairs(info)do 
        message = message .. v .. "      "
    end 
    local file = debug.getinfo(3).short_src
    local line = debug.getinfo(3).currentline
    log(name,level,file,line,message)
end 
---------------日志器--------------------
class("Logger")

function Logger:ctor(name)
    self._name = name
end 

function Logger:log_debug(...)
    c_log(self._name,LOG_TYPE.DEBUG,...)
end 

function Logger:log_info(...)
    c_log(self._name,LOG_TYPE.INFO,...)
end 

function Logger:log_warn(...)
    c_log(self._name,LOG_TYPE.WARN,...)
end 

function Logger:log_error(...)
    c_log(self._name,LOG_TYPE.ERROR,...)
end 

function Logger:log_fatal(...)
    c_log(self._name,LOG_TYPE.FATAL,...)
end 

---------------日志管理器--------------------
single("LoggerMgr")
function LoggerMgr:getLogger(name)
    if not self._loggers then 
        self._loggers = {}
    end 
    if self._loggers[name] then 
        return self._loggers[name]
    else 
        local logger = Logger.new(name)
        self._loggers[name] = logger 
        return logger
    end 
end 

--hook断言
local _assert = assert 
assert = function(v,logger,...)
    if not v then 
        local messages = {...}
        local message = "    "
        for k,v in pairs(messages)do 
            message = message .. v .. "    "
        end 
        message = message .. debug.traceback("", 2)
        local file = debug.getinfo(3).short_src
        local line = debug.getinfo(3).currentline
        log(logger._name,LOG_TYPE.ERROR,file,line,message)
        error()
    end 
end 



