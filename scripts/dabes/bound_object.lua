-- BoundObject
--
-- Some weird meta stuff that lets us have a nice object oriented interface
-- around the C bindings.
local inspect = require 'lib.inspect'
require 'dabes.object'

function map_real(...)
    local uds = {}
    for i = 1, select("#", ...) do
        uds[i] = select(i, ...).real
    end
    return unpack(uds)
end

BoundObject = Object:extend({
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
        bound.born_at = os.time()
        bound:init()

        return bound
    end,

-- Class Functions, e.g. BoundObject.f(...)

    -- fwd_func
    --
    -- Used to set up convenient forwarding from Lua interface into binding.
    fwd_func = function(name)
        return function(self, ...)
            if not self.real then return nil end

            return self.real[name](self.real, ...)
        end
    end,

    -- fwd_adder
    --
    -- Used to set up convenient forwarding from Lua interface into binding
    -- that adds an element to an array. It will cache the Lua instance of
    -- the element so it doesn't get collected prematurely.
    fwd_adder = function(name)
        return function(self, member, ...)
            if not self.real then return nil end
            local fwded = self.real[name]

            local cachekey = '_'..name..'_cache'
            if self[cachekey] == nil then rawset(self, cachekey, {}) end
            -- Hold on to the member so it isn't collected
            self[cachekey][member] = true

            return fwded(self.real, member, ...)
        end
    end,

    -- fwd_remover
    --
    -- Used to set up convenient forwarding from Lua interface into binding
    -- that removes an element from an array. It will remove the cached instance
    -- of an element so it can be collected.
    fwd_remover = function(name)
        return function(self, member, ...)
            if not self.real then return nil end
            local fwded = self.real[name]

            local cachekey = '_'..name..'_cache'
            if self[cachekey] == nil then rawset(self, cachekey, {}) end
            self[cachekey][member] = nil

            return fwded(self.real, member, ...)
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
