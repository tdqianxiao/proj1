local flist = {}
local tm0
function log(...)
	local tma = tm0
	local tmb = ltime_real_ms()
	local dif = tmb - tma
	tm0 = tmb
	print('time:'..(dif),...)
end

if _WIN32 then
	function jobs_begin(path_base)
		tm0 = ltime_real_ms()
		log('flist bulding...')
		local fd = io.popen(string.format('c.bat %s\\xlsx',path_base))
		local ss = fd:read('*all')
		fd:close()
		log('md5 bulding...')
		for path in string.gmatch(ss,'xlsx\\(%S+).xlsx%s*\n') do
			local f1 = path_base..'\\xlsx\\'..path..'.xlsx'
			if not string.find(f1,'[~][$]') then
				local m1 = file_md5(f1)
				--print(full,md5)
				assert(m1)
				local f2 = path_base..'\\l-xlsx\\'..path..'.lua'
				local m2 = tostring( file_md5(f2) )--can be nil
				--print(full2,md52)
				flist[path] = {
					m1 = m1,
					m2 = m2,
					f1 = f1
				}
			end
		end
		log('md5 compairing...')
		local fd = io.open('fmd5.txt','rb')
		if fd then
			for ln in fd:lines() do
				local path, m1, m2 = string.match(ln,'(%S+)%s+(%S+)%s+(%S+)')
				if path and m1 and m2 then
					path = string.gsub(path,'/','\\')
					path = string.gsub(path,'[.]','\\')
					--local full = path_base..'\\xlsx\\'..path..'.xlsx'
					local c = flist[path]
					if c then
						if c.m1 == m1 and c.m2 == m2 then
							c.ignore = 1
						end
					end
				end
			end
			fd:close()
		end
		log('transform deploying...')
		for path,c in pairs(flist) do
			if not c.ignore then
				c.job = jobs_set(c.f1)
				assert(c.job)
			end
		end
		log('transform begin...')
	end

	function jobs_end()
		log('transform finished')
		local fd = io.open('fmd5.txt','wb')
		assert(fd)
		local farr = {}
		for path,c in pairs(flist) do
			table.insert(farr,path)
		end
		table.sort(farr)
		for i,path in ipairs(farr) do
			local c = flist[path]
			if not c.ignore then
				assert(c.job)
				local f1,m2 = jobs_get(c.job)
				assert(c.f1 == f1)
				c.m2 = m2
			end
			path = string.gsub(path,'\\','.')
			path = string.gsub(path,'/','.')
			fd:write(string.format('%s %s %s\r\n',path,c.m1,tostring(c.m2)))
		end
		fd:close()
	end
else
	function jobs_begin(path_base)
		tm0 = ltime_real_ms()
		log('flist bulding...',string.format('sh c.sh %s',path_base))
		local fd = io.popen(string.format('sh c.sh %s',path_base))
		local ss = fd:read('*all')
		fd:close()
		log('md5 bulding...')
		for ln in string.gmatch(ss,'(%S-)\n') do
			local path = string.match(ln,'xlsx/(%S+)[.]xlsx')
			if path then
				local f1 = path_base..'/xlsx/'..path..'.xlsx'
				if not string.find(f1,'[~][$]') then
					local m1 = file_md5(f1)
					--print(full,md5)
					assert(m1)
					local f2 = path_base..'/l-xlsx/'..path..'.lua'
					local m2 = tostring( file_md5(f2) )--can be nil
					--print(full2,md52)
					flist[path] = {
						m1 = m1,
						m2 = m2,
						f1 = f1
					}
				end
			end
		end
		log('md5 compairing...')
		local fd = io.open('fmd5.txt','rb')
		if fd then
			for ln in fd:lines() do
				local path, m1, m2 = string.match(ln,'(%S+)%s+(%S+)%s+(%S+)')
				if path and m1 and m2 then
					path = string.gsub(path,'\\','/')
					path = string.gsub(path,'[.]','/')
					--local full = path_base..'\\xlsx\\'..path..'.xlsx'
					local c = flist[path]
					if c then
						if c.m1 == m1 and c.m2 == m2 then
							c.ignore = 1
						end
					end
				end
			end
			fd:close()
		end
		log('transform deploying...')
		for path,c in pairs(flist) do
			if not c.ignore then
				c.job = jobs_set(c.f1)
				assert(c.job)
			end
		end
		log('transform begin...')
	end

	function jobs_end()
		log('transform finished')
		local fd = io.open('fmd5.txt','wb')
		assert(fd)
		local farr = {}
		for path,c in pairs(flist) do
			table.insert(farr,path)
		end
		table.sort(farr)
		for i,path in ipairs(farr) do
			local c = flist[path]
			if not c.ignore then
				assert(c.job)
				local f1,m2 = jobs_get(c.job)
				assert(c.f1 == f1)
				c.m2 = m2
			end
			path = string.gsub(path,'\\','.')
			path = string.gsub(path,'/','.')
			fd:write(string.format('%s %s %s\r\n',path,c.m1,tostring(c.m2)))
		end
		fd:close()
	end
end