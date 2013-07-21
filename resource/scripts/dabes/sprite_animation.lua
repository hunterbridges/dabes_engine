--- Represents a timed sequence of @{sprite|Sprite} frames
--
-- @{sprite_animation|SpriteAnimation} extends @{bound_object|BoundObject}
--
-- @module sprite_animation
-- @type SpriteAnimation

require 'dabes.bound_object'

SpriteAnimation = BoundObject:extend({
    lib = dab_spriteanimation,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- The current frame index of the animation. This is relative to
        -- the animation, not the sprite.
        --
        -- Index `0` is the first frame in the animation.
        current_index = BoundObject.fwd_func('get_current_index'),

        --- The rate the animation advances in frames per second.
        fps = BoundObject.fwd_func('get_fps'),

        --- **(bool)** Whether the animation should repeat.
        --
        -- Default `true`
        repeats = BoundObject.fwd_func('get_repeats')
    },

    _setters = {
        current_index = BoundObject.fwd_func('set_current_index'),
        fps = BoundObject.fwd_func('set_fps'),
        repeats = BoundObject.fwd_func('set_repeats')
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `SpriteAnimation` from a sequence of frames.
    --
    -- @function SpriteAnimation:new
    -- @tparam number ... A variable list of @{sprite|Sprite} frames to
    -- compose the animation with.
    -- @treturn SpriteAnimation
    -- @usage local anim = SpriteAnimation:new(1, 2, 3, 2) -- Walking
    realize = function(class, ...)
        return class.lib.new(...)
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when `SpriteAnimation` reaches the end.
    --
    -- This is not called if `repeats` is `true`
    --
    -- @tparam SpriteAnimation self The `SpriteAnimation` instance
    complete = function(self)
    end
})

