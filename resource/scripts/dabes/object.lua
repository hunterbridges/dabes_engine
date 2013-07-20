--- @{object|Object}
--
-- The base class for all objects.
-- @type Object

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

--- Configuration.
-- @section configuration

    --- A table of functions that can be used to intercept read access to
    -- fields of an instance.
    --
    -- The functions should take two arguments, `self` and `key`.
    --
    -- When `instance.foo` is accessed, `_getters` is checked for the
    -- field `foo`. If a function is stored there, it is called, and the
    -- return value is passed up. This works via the `__index` metamethod.
    --
    -- Under the hood, this: `instance.foo`
    --
    -- Turns into something like `instance._getters.foo(instance, foo)`
    _getters = nil,

    --- A table of functions that can be used to intercept write access to
    -- fields of an instance.
    --
    -- The functions should take three arguments, `self`, `key`, and `val`.
    --
    -- When `instance.foo` is assigned to, `_setters` is checked for the
    -- field `foo`. If a function is stored there, it is called, and the
    -- return value is passed up. This works via the `__newindex` metamethod.
    --
    -- Under the hood, this: `instance.foo = "bar"`
    --
    -- Turns into something like `instance._setters.foo(instance, foo, "bar")`
    _setters = nil,

--- Class Methods.
-- @section classmethods

    --- Create a new instance of `Object`
    -- @param ... Constructor parameters
    -- @name Object:new
    -- @treturn Object A new instance of `Object`
    new = function(...)
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

    --- Create a new class by extending `Object`
    -- @name Object:extend
    -- @tparam table sub A table of new properties to extend superclass with
    -- @treturn Object A new class with the extended properties.
    extend = function(super, sub)
        setmetatable(sub, super)
        sub.__index = sub

        return sub
    end,

--- Properties
-- @section properties

    --- General identity property for ```Object```
    -- This is ```true``` for all Objects.
    _isobject = true,

    -- Added this at 5 AM
    istypeof = typeof
}
Object.__index = Object
