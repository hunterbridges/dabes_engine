--- Represents an individual internet multiplayer game.
--
-- @{net_match|NetMatch} extends @{bound_object|BoundObject}
--
-- @module controller
-- @type Controller

require 'dabes.bound_object'

NetMatch = BoundObject:extend({
    lib = dab_net_match,

    realize = function(class, ...)
        return class.lib.new(...)
    end
})
