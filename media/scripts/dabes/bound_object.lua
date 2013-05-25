-- BoundObject
--
-- Some weird meta stuff that lets us have a nice object oriented interface
-- around the C bindings.
require 'dabes.object'

BoundObject = Object:extend({
-- Default Configuration

    -- real
    --
    -- Holds the userdata that represents the binding instance.
    real = nil,

-- Class Methods, e.g. BoundObject:method(...)

    -- new
    --
    -- This is the external method used to create a bound object.
    -- It calls the realize hook, then the init hook.
    new = function(class)
        local bound = {}
        setmetatable(bound, class)

        bound.real = bound:realize()
        bound:init()

        return bound
    end,

-- Class Functions, e.g. BoundObject.f(...)

    -- fwd_func
    --
    -- Used to set up convenient forwarding from Lua interface into binding.
    fwd_func = function(name)
        return function(self, ...)
            if not self.real then
                return nil
            end

            return self.real[name](self.real, ...)
        end
    end,

-- Hooks

    -- realize
    --
    -- This should interact with the binding and return a userdata that
    -- is then stored in self.real
    realize = function(self) end,

    -- init
    --
    -- Hook called immediately after `realize`. This is only called once
    -- in the bound object's lifecycle.
    init = function(self) end

})
