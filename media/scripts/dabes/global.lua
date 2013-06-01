-- Global
--
-- Making the Lua global env do weird stuff

print "global should fire first"

_retains = {}
setmetatable(_retains, {_mode="v"})
function retain(obj)
    if _retains[obj] == nil then
        _retains[obj] = 1
        print("retained", obj)
    else
        _retains[obj] = _retains[obj] + 1
    end
    return obj
end

function release(obj)
    if _retains[obj] == nil then return end
    _retains[obj] = _retains[obj] - 1
    if _retains[obj] == 0 then _retains[obj] = nil end
    return obj
end

_veil = {}
_injector = {}
setmetatable(_G, _injector)
_injector.__index = _veil

rawset(_injector, "__newindex", function(table, key, val)
    local exists = table._veil[key]
    if exists ~= nil and type(exists) == "table" then
        -- If it already exists, soft copy the members so they get
        -- injected into the existing instance's metatables
        print("injecting", val, "to global", key)
        for k, v in pairs(val) do
            exists[k] = v
        end
    else
        print("setting", val, "to global", key)
        table._veil[key] = val
    end
end)
