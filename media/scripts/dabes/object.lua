-- Object
--
-- Some API wrangling to inherit behavior and allow for getters and setters.

Object = {
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
Object.__index = Object
