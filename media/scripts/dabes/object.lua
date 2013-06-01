-- Object
--
-- Some API wrangling to inherit behavior and allow for getters and setters.

require 'dabes.global'

Object = {
    new = function(class)
        local minstance = {
            __index = function(self, key)
                local meta = getmetatable(self)
                local raw = meta[key]
                if raw ~= nil then
                    return raw
                end

                local getters = meta._getters
                if getters ~= nil and getters[key] ~= nil then
                    local got = getters[key](self)
                    self._cache[key] = got
                    return got
                else
                    return nil
                end
            end,

            __newindex = function(self, key, value)
                local meta = getmetatable(self)
                local setters = meta._setters
                if setters ~= nil and setters[key] ~= nil then
                    self._cache[key] = value
                    setters[key](self, value)
                else
                    rawset(self, key, value)
                end
            end,

            _cleancache = function(self)
                print("cleaning cache for", self)
                self._cache = {}
            end
        }
        local instance = {}

        setmetatable(minstance, class)
        setmetatable(instance, minstance)
        instance:_cleancache()
        return instance
    end,

    extend = function(super, sub)
        setmetatable(sub, super)
        sub.__index = sub

        return sub
    end,

    _isobject = true
}
Object.__index = Object
