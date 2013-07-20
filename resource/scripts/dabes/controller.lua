--- An interface for receiving input from the player.
--
-- Use `get_controller` to obtain controller instances that receive data from
-- the game engine.
--
-- @{controller|Controller} extends @{bound_object|BoundObject}
--
-- @module controller
-- @type Controller

require 'dabes.bound_object'

Controller = BoundObject:extend({
    lib = dab_controller,

    realize = function(class, i)
        return class.lib.new(i)
    end,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- **(bool)** *(read only)* Up direction is pressed.
        up = BoundObject.fwd_func('is_up'),

        --- **(bool)** *(read only)* Down direction is pressed.
        down = BoundObject.fwd_func('is_down'),

        --- **(bool)** *(read only)* Left direction is pressed.
        left = BoundObject.fwd_func('is_left'),

        --- **(bool)** *(read only)* Right direction is pressed.
        right = BoundObject.fwd_func('is_right'),

        --- **(bool)** *(read only)* A button is pressed.
        a_button = BoundObject.fwd_func('is_jumping')
    },

    _setters = {
        up = BoundObject.readonly,
        down = BoundObject.readonly,
        left = BoundObject.readonly,
        right = BoundObject.readonly,
        a_button = BoundObject.readonly
    },

    _noinject = true
})

--- Global Functions.
-- @section functions

_controllers = {}

--- Obtain a controller instance that is registered to receive data from the
-- game engine.
-- @tparam number i Which controller slot to grab. Currently supports `1` - `4`
-- @treturn `Controller` the registered instance
function get_controller(i)
    if _controllers[i] ~= nil then return _controllers[i] end
    _controllers[i] = Controller:new(i)
    return _controllers[i]
end
