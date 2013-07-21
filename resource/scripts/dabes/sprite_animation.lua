--- Represents a timed sequence of @{sprite|Sprite} frames
--
-- @{sprite_animation|SpriteAnimation} extends @{bound_object|BoundObject}
--
-- @module sprite_animation
-- @type SpriteAnimation

require 'dabes.bound_object'

SpriteAnimation = BoundObject:extend({
    lib = dab_spriteanimation,

-- Hook Overloads
    realize = function(class, ...)
        return class.lib.new(...)
    end,

-- Bound Functions
    _getters = {
        current_index = BoundObject.fwd_func('get_current_index'),
        fps = BoundObject.fwd_func('get_fps'),
        repeats = BoundObject.fwd_func('get_repeats')
    },

    _setters = {
        current_index = BoundObject.fwd_func('set_current_index'),
        fps = BoundObject.fwd_func('set_fps'),
        repeats = BoundObject.fwd_func('set_repeats')
    },

-- Hooks

    -- complete
    --
    -- Hook called when an animation reaches the end.
    complete = function(self)
    end
})

