local Utils = {}

--两点距离
function Utils.distance(x1, y1, x2, y2)
	local a = mathAbs(x1 - x2)
	local b = mathAbs(y1 - y2)
	return math.sqrt(a*a + b*b)
end

-- 树型打印一个 table,不用担心循环引用
table.print = function(root,depthMax,excludeKey,excludeType)
	assert(type(root)=="table","无法打印非table")
	depthMax = depthMax or 3 -- 默认三层
	local cache = { [root] = "." }
	local depth = 0
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

					-- table 深度判断
					depth = depth + 1
					if depth>=depthMax or (excludeKey and excludeKey==k) then
						table.insert(temp,space .. "["..key.."]" .." = " .. "{ ... }")
					else
						local tableStr = _dump(v,space .. (next(t,k) and "|" or " ") .. string.rep(" ",#key<4 and 4 or #key),new_key)
						if tableStr then		-- 非空table
							table.insert(temp,space .. "["..key.."]" .." = " .. "{")
							table.insert(temp, tableStr)
							table.insert(temp,space .."},")
						else 						-- 空table
							table.insert(temp,space .. "["..key.."]" .." = " .. "{ },")
						end
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

table.clear = function(t)
	if type(t) == 'table' then
		for k, v in pairs(t) do
			t[k] = nil
		end
	end
end

table.len = function(t)
	if type(t) == "table" then
		local n = 0
		for k, v in pairs(t) do
			n = n + 1
		end
		return n
	else
		assert(nil, type(t))
	end
end

table.copy = function(t)
	local b = {}
	for k, v in pairs(t) do
		b[k] = v
	end
	return b
end

-- 判断table是否是纯数组的table
table.isArray = function(t)
	if type(t) ~= "table" then return end

	local n = #t
	for i,v in pairs(t) do
		if type(i) ~= "number" then return end
		if i > n then return end
	end

	return true
end

table.cmp = function(t1,t2)
	local function _cmp(t1,t2)
		if type(t1)=="table" and type(t2)=="table" then
			if table.len(t1)~=table.len(t2) then
				return -1
			end

			for k,v in pairs(t1) do
				if type(v)=="table" then
					local ret = _cmp(v,t2[k])
					if ret~=0 then
						return -1
					end
				else
					if v~=t2[k] then
						return -1
					end
				end
			end

			return 0
		end
		return -1
	end

	return _cmp(t1,t2)
end