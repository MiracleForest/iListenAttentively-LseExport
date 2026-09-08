--[[
    通用有序 JSON 编码
    tbl    : 要序列化的 Lua 表
    rules  : 嵌套顺序规则表，格式如：
             {
                 [""]                       = { "key1", "key2", ... },  -- 根对象的键顺序
                 ["path.to[]"]              = { "subkey1", "subkey2" },  -- 某个数组元素对象的键顺序
                 ["path.to.object"]         = { ... },                  -- 某个子对象的键顺序
                 -- 未指定的层级默认按字母升序排列
             }
    indent : 缩进空格数，默认 4
    返回   : 格式化 JSON 字符串
--]]
function ordered_json_encode(tbl, rules, indent)
    indent = indent or 4
    rules = rules or {}

    local function encode(value, level, path)
        local value_type = type(value)

        if value_type == "nil" then return "null"
        elseif value_type == "boolean" then return tostring(value)
        elseif value_type == "number" then return tostring(value)
        elseif value_type == "string" then return string.format("%q", value)
        elseif value_type == "table" then
            local is_array = true
            local max_index = 0
            local count = 0
            for k in pairs(value) do
                if type(k) ~= "number" or math.floor(k) ~= k or k < 1 then
                    is_array = false
                    break
                end
                if k > max_index then max_index = k end
                count = count + 1
            end
            if is_array and max_index == count then
                local parts = {}
                for i, item in ipairs(value) do
                    local child_path = path .. "[]"
                    parts[#parts + 1] = string.rep(" ", (level + 1) * indent) .. encode(item, level + 1, child_path)
                end
                return "[\n" .. table.concat(parts, ",\n") .. "\n" .. string.rep(" ", level * indent) .. "]"
            else
                local order = rules[path]
                if not order then
                    order = {}
                    for k in pairs(value) do
                        order[#order + 1] = k
                    end
                    table.sort(order)
                end

                local parts = {}
                for _, key in ipairs(order) do
                    if value[key] ~= nil then
                        local key_json = string.format("%q", key)
                        local child_path = (path == "" and key) or (path .. "." .. key)
                        local value_json = encode(value[key], level + 1, child_path)
                        parts[#parts + 1] = string.rep(" ", (level + 1) * indent) .. key_json .. ": " .. value_json
                    end
                end
                return "{\n" .. table.concat(parts, ",\n") .. "\n" .. string.rep(" ", level * indent) .. "}"
            end
        else
            return "null"
        end
    end

    return encode(tbl, 0, "")
end