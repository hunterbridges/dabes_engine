-- Object
--
-- Some API wrangling to inherit behavior and allow for getters and setters.

require 'dabes.global'

-- Added this at 5 AM
-- iskindof(obj, "Entity")
iskindof = function(obj, tstr)
    return obj["_is_"..tstr] == true
end

-- TODO: Use this to typecheck object methods.
assert_method = function(obj, tstr)
    if iskindof(obj, tstr) ~= true then
        error("Expecting "..tstr..". Accessing the method with . ?", 4)
    end
end

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

    _isobject = true,

    -- Added this at 5 AM
    istypeof = typeof
}
Object.__index = Object
