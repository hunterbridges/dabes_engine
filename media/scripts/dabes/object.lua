-- Object
--
-- Some API wrangling to inherit behavior and allow for getters and setters.

_ = {_veil = {}}
_injector = {
    __newindex = function(table, key, val)
        exists = table._veil[key]
        print "testing"
        if exists ~= nil then
            -- If it already exists, soft copy the members so they get
            -- injected into the existing instance's metatables
            for k, v in pairs(val) do
                print("Injecting", k)
                exists[k] = v
            end
        else
            table._veil[key] = val
        end
    end,
    
    __index = function(table, key)
      return table._veil[key]
    end
}
setmetatable(_, _injector)

_.Object = {
    new = function(class)
        local minstance = {
            __index = function(self, key)
                meta = getmetatable(self)
                raw = meta[key]
                if raw ~= nil then
                    return raw
                end

                getters = meta._getters
                if getters ~= nil and getters[key] ~= nil then
                    return getters[key](self)
                else
                    return nil
                end
            end,

            __newindex = function(self, key, value)
                meta = getmetatable(self)
                setters = meta._setters
                if setters ~= nil and setters[key] ~= nil then
                    setters[key](self, value)
                else
                    rawset(self, key, value)
                end
            end
        }
        local instance = {}

        setmetatable(minstance, class)
        setmetatable(instance, minstance)
        return instance
    end,

    extend = function(super, sub)
        setmetatable(sub, super)
        sub.__index = sub

        return sub
    end
}
Object = _.Object
Object.__index = Object
