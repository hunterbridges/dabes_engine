-- Global
--
-- Making the Lua global env do weird stuff

math.randomseed(os.time())

_veil = {}
_injector = {}
setmetatable(_G, _injector)
_injector.__index = _veil

rawset(_injector, "__newindex", function(table, key, val)
    local exists = table._veil[key]
    if (exists ~= nil and type(exists) == "table" and exists._isobject
        and exists._noinject ~= true) then
        -- If it already exists, soft copy the members so they get
        -- injected into the existing instance's metatables
        for k, v in pairs(val) do
            exists[k] = v
        end
    else
        table._veil[key] = val
    end
end)
