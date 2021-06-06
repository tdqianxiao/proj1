single("Config")
local logger = LoggerMgr:getLogger("root")

local files = {
    "config.l-xlsx.misc",
}

function Config:loadConfig()
    self._datas = {}
    for k,v in pairs(files)do 
        local info = require(v)
        local profixLen = string.len("config.l-xlsx.")
        local filenameLen = string.len(v)
        local key = string.sub(v,profixLen + 1,filenameLen)
        self._datas[key] = info 
    end
end 

function Config:find(name,id)
    if not self._datas[name] then 
        logger:log_warn(name,"not found")
        return 
    end 
    if not self._datas[name][1][id] then 
        logger:log_warn(name..".id:"..id.." not found")
    end 
    return self._datas[name][1][id]
end 

