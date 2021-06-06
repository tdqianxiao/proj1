local core = require('lcore')
local translate = require('translate')
local fbuf = fbuf_create()
local json = require('json')
local function error(str)
	if (_WIN32) then 
		os.execute("echo [31m ".. str .." [0m")
	else
		os.execute("echo -e '\\033[31m " .. str ..  "\\033[0m'")
	end
end


local function write_data22(ctx,data)
	--[[if not ctx.fd then
		os.execute(string.format('IF NOT EXIST "%s" MD "%s"',ctx.pdir,ctx.pdir))
		ctx.fd = io.open(ctx.pdst,'wb')
	end
	local fd = ctx.fd
	fd:write('{\n')]]
	fbuf_write(fbuf,string.format('	[%s] = {\n',data[2]))
	for i = 1, #data, 2 do
		local k = data[i]
		local v = data[i+1]
		if type(v) == 'string' then
			--fd:write(
				fbuf_write(fbuf, string.format('		["%s"] = "%s",\n',k,v) )
			--)
		else
			--fd:write(
				fbuf_write(fbuf, string.format('		["%s"] = %s,\n',k,v) )
			--)
		end
	end
	--fd:write('}\n')
	fbuf_write(fbuf,'},\n')
end

function _work2(psur,pdst,pdir)
	--if not string.find(psur,'liveness.xlsx') then return end --TEST
	fbuf_reset(fbuf);
	local col2name = {}
	local col2type = {}
	local col2side = {}
	local col2type2 = {}
	local col2seq = {}
	local jsonfield = {}
	local server_siden = 0
	local rown = 0
	local sheets = core.load_xlsx(psur)
	local head = nil
	local flag
	local id_set = {}
	local lang = translate.load(psur)
	local current_row_id
	for _,v in pairs(sheets) do
		local name = v.name
		local current_row
		local cancel
		local sidesn = 0
		local ctx = {psur=psur,pdst=pdst,pdir=pdir}
		for _,v in ipairs(v.sheetData) do
			local value = core.escape_value(v.v) --v.f?function ?
			local col,row = string.match(v.r,'(%a+)(%d+)')
			row = assert(tonumber(row))
			if row == 1 then
				--do nothing
			elseif row == 2 then
				col2name[col] = value
				table.insert(col2seq,col)
			elseif row == 3 then
				if value == 'int' or value == 'number' then
					col2type[col] = tonumber
				elseif value == 'string' then
					col2type[col] = tostring
				elseif value == 'object' or value == 'array' or value == 'table' then
					col2type[col] = 'tostring'
					jsonfield[col] = true
				end
				col2type2[col] = value
			elseif row == 4 then
				col2side[col] = value
				if value == 's' or value == 'd' then
					sidesn = sidesn + 1
				end
			else
				if sidesn == 0 then
					cancel = true
					break
				else
					if col2type[col] == tostring then
						value = string.gsub(value,'\\r\\n','\r\n')
						value = string.gsub(value,'\\n','\r\n')
						value = string.gsub(value,"'","\\'")
					end					
					if not current_row or current_row ~= row then
						if current_row then
							if not flag then--上一行是有效行
								fbuf_write(fbuf,'	},\n')
							end
						else
							fbuf_write(fbuf,"--Don't Edit!!!\nreturn {\n{\n")
							head = true
						end
						current_row = row
						flag = nil
						current_row_id = nil
						
						local vvvv = tonumber(value)
						if vvvv and col2name[col] == 'id' then
							if id_set[vvvv] then
								error(string.format('ERROR: id repeated:%d file:%s',vvvv, psur))
								assert()
							end
							id_set[vvvv] = true
							fbuf_write(fbuf,"	[")
							fbuf_write(fbuf,vvvv)
							fbuf_write(fbuf,"] = {\n")
							rown = rown + 1
							current_row_id = vvvv
						else
							flag = true
						end
						--print(111,col2name[col],flag,not flag and col2side[col] == 's' or col2side[col] == 'd')
					end
					if not flag and (col2side[col] == 's' or col2side[col] == 'd') then
						if col2type[col] == tonumber then
							value = tonumber(value)
						else
							if string.len(value) == 0 then
								value = nil
							end
						end
						if value then
							if jsonfield[col] then
								local err, obj = pcall(function() return json.decode(value) end)
								if not err then
									assert(err,string.format('json error ! file:%s id:%s field:%s  val:%s',psur,current_row_id or '',col2name[col] or '',value))
								end
								
							end
							fbuf_write(fbuf, "		['")
							fbuf_write(fbuf, col2name[col])
							fbuf_write(fbuf, "'] = ")
							if col2type[col] == tonumber then-- or col2type[col] == 'number' then
								fbuf_write(fbuf,value)
								fbuf_write(fbuf,',\n')
							else
								fbuf_write(fbuf,"'")
								--start translate
								if lang then
									assert(current_row_id)
									local rowobj = lang[current_row_id]
									if rowobj then
										local tvalue = rowobj[col2name[col]]
										if tvalue and tvalue ~= '' then
											value = tvalue
										end
									end
								end
								--end translate
								fbuf_write(fbuf,value)
								fbuf_write(fbuf,"',\n")
							end
						end
					end
				end
			end
			if cancel then
				break
			end
		end
		
		if not cancel then
			if rown > 0 then
				if not flag then--是有效行
					fbuf_write(fbuf,'	},\n')
				end
			elseif not head then
				fbuf_write(fbuf,"--Don't Edit!!!\nreturn {\n{\n")
			end
			fbuf_write(fbuf,'},\n\n{\n')
			for _, col in pairs(col2seq) do
				if col2side[col] == 's' or col2side[col] == 'd' then
					fbuf_write(fbuf,"	['")
					fbuf_write(fbuf,col2name[col])
					fbuf_write(fbuf,"'] = '")
					fbuf_write(fbuf,col2type2[col])
					fbuf_write(fbuf,"',\n")
				end
			end
			fbuf_write(fbuf,"},\n\n'id'\n}\n")
			fbuf_write(fbuf,string.format('--rown=%s\n',rown))
			local md5 = fbuf_md5(fbuf)
			fbuf_flush(fbuf,pdst,pdir)
			return md5
		end
	end
end

function _work(ln,tid)
	if string.find(ln,'~[$]') then return end--打开状态的临时文件 无视 比如 ~$activeIcon.xlsx
	local p1,p2 = string.match(ln,'(%S-)\\xlsx\\(%S-).xlsx')
	if p1 and p2 then--and string.match(ln,'ge_upgrade_bill.xlsx')then
		--assert()
		local psur = p1..'\\xlsx\\'..p2..'.xlsx'
		local pdst = p1..'\\l-xlsx\\'..p2..'.lua'
		local pdir = string.match(pdst,'(%S+)\\%S+.lua')
		print(string.format('thread:%3d loading %s',tid,psur))
		_work2(psur,pdst,pdir)
		--assert(nil,ln)
	end
	
end

function _workWin(ln,tid)
	_WIN32 = true
	local p1,p2 = string.match(ln,'(%S-)\\xlsx\\(%S-).xlsx')
	--[[print(111)
	print(ln)
	print(p1)
	print(p2)
	print(222)]]
	if p1 and p2 then--and string.match(ln,'ge_upgrade_bill.xlsx')then
		--assert()
		local psur = p1..'\\xlsx\\'..p2..'.xlsx'
		local pdst = p1..'\\l-xlsx\\'..p2..'.lua'
		local pdir = string.match(pdst,'(%S+)\\%S+.lua')
		print(string.format('thread:%3d loading %s',tid,psur))
		local md5 = _work2(psur,pdst,pdir)
		--print(md5)
		collectgarbage('collect')
		return md5 or 'nil'
		--assert(nil,ln)
	end
	return 'nil'
end

function _workUnix(ln,tid)
	local p1,p2 = string.match(ln,'(%S-)/xlsx/(%S-).xlsx')
	--[[print(111)
	print(ln)
	print(p1)
	print(p2)
	print(222)]]
	if p1 and p2 then--and string.match(ln,'ge_upgrade_bill.xlsx')then
		--assert()
		local psur = p1..'/xlsx/'..p2..'.xlsx'
		local pdst = p1..'/l-xlsx/'..p2..'.lua'
		local pdir = string.match(pdst,'(%S+)/%S+.lua')
		print(string.format('thread:%3d loading %s %s',tid,psur,ln))
		local md5 = _work2(psur,pdst,pdir)
		--print(md5)
		collectgarbage('collect')
		return md5 or 'nil'
		--assert(nil,ln)
	end
	return 'nil'
end