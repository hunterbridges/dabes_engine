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
        local realized = class.lib.new(i)

        return realized
    end,
    
    init = function(self)
        self.pressed = {
            up = false,
            down = false,
            left = false,
            right = false,
            a_button = false
        }
    
        self.released = {
            up = false,
            down = false,
            left = false,
            right = false,
            a_button = false
        }

        self.prev = {
            up = false,
            down = false,
            left = false,
            right = false,
            a_button = false
        }
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

    --- A table describing which buttons were pressed down on the current
    -- frame. For example, `controller.pressed.up`
    pressed = nil,

    --- A table describing which buttons were released on the current
    -- frame. For example, `controller.released.up`
    released = nil,


    update = function(self)
        local prev = self.prev

        self.pressed.up = false
        self.pressed.down = false
        self.pressed.left = false
        self.pressed.right = false
        self.pressed.a_button = false

        self.released.up = false
        self.released.down = false
        self.released.left = false
        self.released.right = false
        self.released.a_button = false

        if self.up and not prev.up then self.pressed.up = true end
        if self.down and not prev.down then self.pressed.down = true end
        if self.left and not prev.left then self.pressed.left = true end
        if self.right and not prev.right then self.pressed.right = true end
        if self.a_button and not prev.a_button then
            self.pressed.a_button = true
        end

        if prev.up and not self.up then self.released.up = true end
        if prev.down and not self.down then self.released.down = true end
        if prev.left and not self.left then self.released.left = true end
        if prev.right and not self.right then self.released.right = true end
        if prev.a_button and not self.a_button then
            self.released.a_button = true
        end
        
        prev.up = self.up
        prev.down = self.down
        prev.left = self.left
        prev.right = self.right
        prev.a_button = self.a_button
    end,

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
