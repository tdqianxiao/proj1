local core = require('lcore')
--[[local fd = io.popen('dir ..\\language')
local s = fd:read('*all')
fd:close()]] local s = ''
local lang_name
for dir in string.gmatch(s,'<DIR>%s+(%S+)') do
	if dir ~= '.' and dir ~= '..' then
		lang_name = dir
		break
	end
end

local lang = {}


function lang.load(full_path) --full_path = 'xlsx\\achv\\liveness.xlsx'
	if not lang_name then return end
	
	local path = string.match(full_path,'xlsx\\(%S+)')
	assert(path)
	path = string.gsub(path,'\\','$')
	local sheetname = '$'..path
	sheetname = string.gsub(sheetname,'.xlsx','')
	path = '..\\language\\'..lang_name..'\\$'..path
	local load_err = nil
	local ok, sheets = xpcall(
		function() 
			return core.load_xlsx2(path,sheetname) 
		end, 
		function(err)
			--print(err)
			load_err = err
		end
	)
	if load_err then return end
	print(string.format('		translated fields from %s...\n',path))
	local col2name = {}
	local current_row
	local current_row
	local current_row_id
	local current_row_tb
	local maps = {}
	for _,v in pairs(sheets) do
		--local name = v.name
		for _,v in ipairs(v.sheetData) do
			local value = core.escape_value(v.v) --v.f?function ?
			local col,row = string.match(v.r,'(%a+)(%d+)')
			row = assert(tonumber(row))
			if current_row ~= row then
				if current_row_tb then
					maps[current_row_id] = current_row_tb
				end
				current_row = row
				current_row_id = nil
				current_row_tb = nil
			end
			if row == 1 then
				col2name[col] = value
			else
				local fieldname = col2name[col]
				if fieldname == 'id' then
					current_row_id = tonumber(value)
					current_row_tb = {}
					assert(current_row_id >= 0)
				else
					local fnm = string.match(fieldname,'(%S+)_翻译')
					if fnm then
						--print(current_row_id, fnm, value)
						current_row_tb[fnm] = value
					end
				end
			end
		end
		if current_row_tb then
			maps[current_row_id] = current_row_tb
		end
		break --once
	end
	--table.print(maps)
	return maps
end

return lang