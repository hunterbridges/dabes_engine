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
require 'lib.json'

NetMatch = BoundObject:extend({
    lib = dab_net_match,

--- Constants.
-- Accessed at the class level e.g. `Class.CONSTANT`
-- @section constants

    --- A `null` message kind. Doesn't contain any data.
    MSG_NULL = 0,

    --- A message kind that contains a @{recorder|Recorder} in the `payload`.
    --
    -- **Sending:** Provide a @{recorder|Recorder} instance in the payload.
    --
    -- **Receiving:** A new @{recorder|Recorder} instance.  It still needs to
    -- be added to a @{scene|Scene} and assigned an @{entity|Entity}.
    MSG_PACKED_RECORDER = 3,

    --- A general-purpose JSON message.
    --
    -- **Sending:** Will encode `payload` into JSON.
    --
    -- **Receiving:** Will decode the JSON and pass the decoded object as
    -- the `payload`.
    MSG_JSON = 4,

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

    --- Fetch metadata for all the players in the match. When the metadata
    -- is retrieved, the `got_metadata` hook is called.
    --
    -- @function net_match:get_metadata
    -- @treturn nil
    get_metadata = BoundObject.fwd_func("get_metadata"),

    --- Send a message to one or all players of the `NetMatch`.
    --
    -- @tparam number to The player number to send the message to. If you pass
    -- `0`, the message will be sent to all players.
    --
    -- @tparam number kind The kind of message to be sent. This may incur
    -- additional type checking for the `payload`
    -- @param payload The contents of the message to be sent. This will be
    -- different depending on the `kind` of message being sent.
    -- @function net_match:handshake
    send_msg = function(self, to, kind, payload)
        if kind == NetMatch.MSG_NULL then
            local fwded = BoundObject.fwd_func("send_null_msg")
            fwded(self, to)
        elseif kind == NetMatch.MSG_PACKED_RECORDER then
            local fwded = BoundObject.fwd_func("send_packed_recorder_msg")

            if not payload:iskindof(Recorder) then
                error("MSG_PACKED_RECORDER requires Recorder "..
                      "in payload.", 2)
            end

            fwded(self, to, payload.real)
        elseif kind == NetMatch.MSG_JSON then
            local fwded = BoundObject.fwd_func("send_json_msg")

            if type(payload) ~= "table" then
                error("MSG_JSON requires table in payload.", 2)
            end

            fwded(self, to, json.encode(payload))
        end
    end,

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

    --- Called when metadata for players has been retrieved in response to
    -- `get_metadata`. This returns metadata with the following structure:
    --
    --     {
    --         {
    --             "display_name" = "foo",
    --             "alias" = "bar"
    --         },
    --         ...
    --     }
    --
    -- The player metadata in the array is indexed by the corresponding
    -- player numbers.
    --
    -- @function got_metadata
    -- @tparam NetMatch self The NetMatch instance
    -- @tparam table metadata An array of player metadata
    got_metadata = function(self, metadata)
    end,

    --- Called when a message is received from another player.
    --
    -- @function got_metadata
    -- @tparam NetMatch self The @{net_match|NetMatch} instance.
    -- @tparam number from The player number of the player who sent the message.
    -- @tparam number kind The kind of the received message.
    -- @param payload The payload of the message.
    receive_msg = function(self)
    end,
})
