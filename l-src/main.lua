require "l-src.include"

class("App")

local logger = LoggerMgr:getLogger("tadpole")

function App:main()
    -- local tab = debug.getinfo(1)
    -- table.print(tab)
    assert(false,logger,"haha")
    --SleepMs(2000)
    --logger:log_info("haha")
    -- Config:loadConfig()
    -- local cnf = Config:find("misc",23)
    -- local val = 0 
    -- val = setBit(val,2,1)
    -- print(getBit(val,2))

    -- print(getBitAmount(-1))

    -- rand(1,100)
end

function App:ctor()
    self:main()
end 

App:new()