-- Global
--
-- Making the Lua global env do weird stuff

require 'lib.inspect'
require 'dabes.util'

math.randomseed(os.time())

_veil = {}
_injector = {}
_globalkeys = newweaktable()
setmetatable(_G, _injector)
_injector.__index = _veil

rawset(_injector, "__newindex", function(table, key, val)
    if val == nil then
        table._veil[key] = nil
        return
    end

    local exists = table._veil[key]
    if _globalkeys[val] == nil then
        if (exists ~= nil and type(exists) == "table" and exists._isobject
            and exists._noinject ~= true) then

            -- If it already exists, soft copy the members so they get
            -- injected into the existing instance's metatables
            for k, v in pairs(val) do
                exists[k] = v
            end
        else
            table._veil[key] = val
            _globalkeys[val] = key

            -- Added this at 5 AM
            if (type(val) == "table" and val._isobject) then
                val["_is_"..key] = true
            end
        end
    else
        if _globalkeys[val] ~= key then
            print("tried to set val to ", key, " but it is already ",
                  _globalkeys[val])
            return
        end
    end

end)
