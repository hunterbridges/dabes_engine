-- BoundObject
--
-- Some weird meta stuff that lets us have a nice object oriented interface
-- around the C bindings.
local inspect = require 'lib.inspect'
require 'dabes.object'

function map_real(...)
    uds = {}
    for i = 1, select("#", ...) do
        uds[i] = select(i, ...).real
    end
    return unpack(uds)
end

_.BoundObject = Object:extend({
-- Default Configuration

    -- lib
    --
    -- The lib that is provided by the game engine.
    lib = nil,

    -- real
    --
    -- Holds the userdata that represents the binding instance.
    real = nil,

-- Class Methods, e.g. BoundObject:method(...)

    -- new
    --
    -- This is the external method used to create a bound object.
    -- It calls the realize hook, then the init hook.
    new = function(class, ...)
        local bound = Object:new()
        local meta = getmetatable(bound)
        setmetatable(meta, class)

        bound.real = bound:realize(...)
        dab_registerinstance(bound.real, bound)
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

    -- readonly
    readonly = function(self, key, val)
        print(key.." is a readonly property")
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
BoundObject = _.BoundObject
