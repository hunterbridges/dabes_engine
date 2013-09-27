--- Represents an individual internet multiplayer game.
--
-- You should never construct this object yourself. It comes from @{net|Net}
-- matchmaking.
--
-- @{net_match|NetMatch} extends @{bound_object|BoundObject}
--
-- @module controller
-- @type Controller

require 'dabes.bound_object'

NetMatch = BoundObject:extend({
    lib = dab_net_match,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- _(readonly)_ The number of players in the match.
        player_count = BoundObject.fwd_func("get_player_count"),

        --- _(readonly)_ The assigned player number of the local player.
        player_number = BoundObject.fwd_func("get_player_number"),
    },

    _setters = {
        player_count = BoundObject.readonly,
        player_number = BoundObject.readonly,
    },

    realize = function(class, ...)
        return class.lib.new(...)
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Begin the player handshake process. This process assigns each player
    -- in the match their `player_number`. When all players have been assigned,
    -- the `all_ready` hook is called.
    --
    -- @function net_match:handshake
    -- @treturn nil
    handshake = BoundObject.fwd_func("handshake"),

    send_msg = BoundObject.fwd_func("send_msg"),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called once all players have informed each other of their player
    -- numbers.
    --
    -- @function all_ready
    -- @tparam NetMatch self The NetMatch instance
    all_ready = function(self)
    end,

    receive_msg = function(self)
    end,
})