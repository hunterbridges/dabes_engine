--- An interface for implementing multiplayer games over the internet.
--
-- @{net|Net} extends @{bound_object|BoundObject}
--
-- @module controller
-- @type Controller

require 'dabes.bound_object'

Net = BoundObject:extend({
    lib = dab_net,

    realize = function(class, ...)
        return class.lib.new(...)
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Authenticate the local player.
    --
    -- @function net:authenticate
    authenticate = BoundObject.fwd_func("authenticate"),

    --- Find matches
    --
    -- @function net:authenticate
    find_matches = BoundObject.fwd_func("find_matches"),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when `Net` authenticates successfully.
    authenticated = function(self)
    end,
    
    --- Called when `Net` finds a match
    found_match = function(self, match)
    end
})

--- Globals.
-- Variables that are stored in the global namespace, usually for special
-- purposes.
-- @section globals

--- The "singleton" instance of `Net`.
net = Net:new()
