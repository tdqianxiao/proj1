table.print = function(root,depthMax,excludeKey,excludeType)
	assert(type(root)=="table","�޷���ӡ��table")
	depthMax = depthMax or 3 -- Ĭ������
	local cache = { [root] = "." }
	local depth = 0
	local print = print
	print("{")
	local function _dump(t,space,name)
		local temp = {}
		for k,v in pairs(t) do
			local key = tostring(k)
			if type(k) == "string" then
				key ='\"' .. tostring(k) .. '\"'
			end

			if type(v) == "table" then
				if cache[v] then
					table.insert(temp,space .. "["..key.."]" .." = " .. " {" .. cache[v].."},")
				else
					local new_key = name .. "." .. tostring(k)
					cache[v] = new_key .." ->[".. tostring(v) .."]"

					-- table ����ж�
					depth = depth + 1
					if depth>=depthMax or (excludeKey and excludeKey==k) then
						table.insert(temp,space .. "["..key.."]" .." = " .. "{ ... }")
					else
						local tableStr = _dump(v,space .. (next(t,k) and "|" or " ") .. string.rep(" ",#key<4 and 4 or #key),new_key)
						if tableStr then		-- �ǿ�table
							table.insert(temp,space .. "["..key.."]" .." = " .. "{")
							table.insert(temp, tableStr)
							table.insert(temp,space .."},")
						else 						-- ��table
							table.insert(temp,space .. "["..key.."]" .." = " .. "{ },")
						end
						--table.insert(temp, _dump(v,space .. (next(t,k) and "|" or " " ).. string.rep(" ",#key),new_key))
					end
					depth = depth -1
				end
			else
				local vType = type(v)
				if not excludeType or excludeType~=vType then
					if vType == "string" then
						v = '\"' .. v .. '\"'
					else
						v = tostring(v) or "nil"
					end
					table.insert(temp,space .. "["..key.."]" .. " = " .. v ..",")
					--tinsert(temp,"+" .. key .. " [" .. tostring(v).."]")
				end
			end
		end

		return #temp>0 and table.concat(temp,"\n") or nil
	end
	local allTableString = _dump(root, "    ","")
	print(allTableString or "")
	print("}")
	return allTableString
end



local zip = require "zip"

---------- xml

-- A modify version from http://lua-users.org/wiki/LuaXml

local xml = {}

local function parseargs(s,arg)
  string.gsub(s, "([%-%w:]+)=([\"'])(.-)%2", function (w, _, a)
    arg[w] = a
  end)
  return arg
end

function xml.collect(s)
  local stack = {}
  local top = {}
  table.insert(stack, top)
  local ni,c,label,xarg, empty
  local i, j = 1, 1
  while true do
    ni,j,c,label,xarg, empty = string.find(s, "<(%/?)([%w:]+)(.-)(%/?)>", i)
    if not ni then break end
    local text = string.sub(s, i, ni-1)
    if text ~= "" then
      if top["xml:space"] == "preserve" then
          table.insert(top, text)
      else
        text = text:match "^%s*(.*)%s*$"
        if text ~= "" then
            table.insert(top, text)
        end
      end
    end
    if empty == "/" then  -- empty element tag
      table.insert(top, parseargs(xarg, {xml=label}))
    elseif c == "" then   -- start tag
      top = parseargs(xarg, { xml=label })
      table.insert(stack, top)   -- new level
    else  -- end tag
      local toclose = table.remove(stack)  -- remove top
      top = stack[#stack]
      if #stack < 1 then
        error("nothing to close with "..label)
      end
      if toclose.xml ~= label then
        error("trying to close "..toclose.xml.." with "..label)
      end
      table.insert(top, toclose)
    end
    i = j+1
  end
  local text = string.sub(s, i)
  if not string.find(text, "^%s*$") then
    table.insert(stack[#stack], text)
  end
  if #stack > 1 then
    error("unclosed "..stack[#stack].label)
  end
  return stack[1]
end

local function seri_args(tbl)
	local tmp = { " " }
	for k,v in pairs(tbl) do
		if type(k) == "string" and k~="xml" then
			table.insert(tmp, string.format('%s="%s"',k,v))
		end
	end
	if #tmp == 1 then
		return ""
	else
		return table.concat(tmp)
	end
end

local function seri(root)
	if root[1] == nil then
		return string.format("<%s%s/>", root.xml, seri_args(root))
	else
		local tmp = { string.format("<%s%s>", root.xml, seri_args(root)) }
		for _,v in ipairs(root) do
			if type(v) == "table" then
				table.insert(tmp, seri(v))
			else
				table.insert(tmp, tostring(v))
			end
		end
		table.insert(tmp, string.format("</%s>", root.xml))
		return table.concat(tmp)
	end
end

function xml.seri(tbl)
	if tbl.xml then
		return seri(tbl)
	else
		local tmp = {}
		for _,v in ipairs(tbl) do
			table.insert(tmp, seri(v))
		end
		return table.concat(tmp)
	end
end

function xml.extract(tbl, style)
	local r = {}
	for _,v in ipairs(tbl) do
		local n = style[v.xml]
		if n then
			if n == true then
				r[v.xml] = v
			else
				r[v.xml] = v[n]
			end
		end
	end

	return r
end

local escape_tbl = setmetatable({
	['&amp;'] = '&',
	['&quot;'] = '"',
	['&lt;'] = '<',
	['&gt;'] = '>',
	['&apos;'] = "'",--eligo mod
}, { __index = function(_,k) assert(k) return k:match"&#(%d+);":char() end })

function xml.unescape(str)
	str = str:gsub("(&[^;]+;)", escape_tbl)
	return str
end

-----------

local function read_xml(self, name)
	local data = self.archive:read(name)
	return xml.collect(data)
end

local function read_sharedstrings(sharedstrings, result)
	assert(sharedstrings and sharedstrings.xml == "sst" , "Invalid sharedstrings.xml")
	for _, v in ipairs(sharedstrings) do
		if v.xml == "si" and v[1] then
			if v[1].xml == "t" then
				if v[1][1] then
					table.insert(result, v[1][1])
				else
					table.insert(result, "")
				end
			else
				-- NOTE: remove font <r> <rPr> ...
				-- http://msdn.microsoft.com/en-us/library/office/gg278314(v=office.15).aspx
				local tmp = {}
				for _, v in ipairs(v) do
					if v.xml == "r" then
						for _, v in ipairs(v) do
							if v.xml == "t" then
								table.insert(tmp, v[1])
							end
						end
					end
				end
				table.insert(result, table.concat(tmp))
			end
		end
	end
end

local SHEET = {}

do
	function SHEET.sheetData(xmltbl, result)
		local data = {}
		for _, v in ipairs(xmltbl) do
			if v.xml == "row" then
				for _,v in ipairs(v) do
					if v.xml == "c" then
						local value = { r = v.r, t = v.t }
						table.insert(data, value)
						for _, v in ipairs(v) do
							value[v.xml] = v[1]
						end
					end
				end
			end
		end
		result.data = data
	end
end

local function parser(tbl, result, method)
	for a, v in ipairs(tbl) do
		local f = method[v.xml]
		if f then
			local r = result[v.xml] or {}
			result[v.xml] = r
			f(v, r)
		end
	end
end

local function load_sheet(self, result)
	local tbl = read_xml(self, "xl/" .. result.filename)
	result.filename = nil
	tbl = tbl[2]
	assert(tbl.xml == "worksheet")

	parser(tbl, result, SHEET)
	result.sheetData, result.sheetData.data = result.sheetData.data

	for _, v in ipairs(result.sheetData) do
		if v.t == "s" then
			v.v = self.sharedstrings[tonumber(v.v)+1]
		end
	end
end

local function load_xlsx(filename)
	local self = {}
	self.archive = assert(zip.unzip(filename), "Can't open " ..  filename)
	self.sharedstrings = {}
	local ok, sharedstrings = pcall(read_xml, self, "xl/sharedStrings.xml")
	if ok then
		read_sharedstrings(sharedstrings[2], self.sharedstrings)
	end

	local sheets = {}
	do
		local workbook = read_xml(self, "xl/workbook.xml")
		workbook = workbook[2]
		assert(workbook.xml == "workbook")
		local workbook_rels = read_xml(self, "xl/_rels/workbook.xml.rels")
		local rels = {}
		assert (workbook_rels[2].xml == "Relationships")
		for _,v in ipairs(workbook_rels[2]) do
			rels[v.Id] = v.Target
		end
		for _,v in ipairs(workbook) do
			-- only support sheets
			if v.xml == "sheets" then
				for _,v in ipairs(v) do
					if v.xml == "sheet" and v.name == 'Sheet1' then --only interest Sheet1
						sheets[tonumber(v.sheetId)] = {
							name = v.name,
							filename = rels[v["r:id"]]
						}
					end
				end
			end
		end
		for _,v in pairs(sheets) do
			load_sheet(self, v)
		end
	end

	self.archive:close()

	return sheets
end


local function load_xlsx2(filename,sheetname)
	local self = {}
	self.archive = assert(zip.unzip(filename), "Can't open " ..  filename)
	self.sharedstrings = {}
	local ok, sharedstrings = pcall(read_xml, self, "xl/sharedStrings.xml")
	if ok then
		read_sharedstrings(sharedstrings[2], self.sharedstrings)
	end

	local sheets = {}
	do
		local workbook = read_xml(self, "xl/workbook.xml")
		workbook = workbook[2]
		assert(workbook.xml == "workbook")
		local workbook_rels = read_xml(self, "xl/_rels/workbook.xml.rels")
		local rels = {}
		assert (workbook_rels[2].xml == "Relationships")
		for _,v in ipairs(workbook_rels[2]) do
			rels[v.Id] = v.Target
		end
		for _,v in ipairs(workbook) do
			-- only support sheets
			if v.xml == "sheets" then
				for _,v in ipairs(v) do
					if v.xml == "sheet" and v.name == sheetname then --only interest 'sheetname'
						sheets[tonumber(v.sheetId)] = {
							name = v.name,
							filename = rels[v["r:id"]]
						}
					end
				end
			end
		end
		for _,v in pairs(sheets) do
			load_sheet(self, v)
		end
	end

	self.archive:close()

	return sheets
end

local opt = {}
local filename

for _, v in ipairs{...} do
	if v:sub(1,1) == "-" then
		opt[v:sub(2)] = true
	else
		filename = v
	end
end

local escape_value_tbl = { ["\\"] = "\\\\" , ["\n"] = "\\n", ["\r"] = "\\r" }
function escape_value(s)
	s = xml.unescape(s or "")
	s = s:gsub("[\\\r\n]", escape_value_tbl)
	return s
end

return {
	load_xlsx = load_xlsx,
	load_xlsx2 = load_xlsx2,
	escape_value = escape_value
}
