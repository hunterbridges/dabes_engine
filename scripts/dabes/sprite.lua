-- Sprite
--
-- This BoundObject represents a Sprite (sprite.h)

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
        fps = BoundObject.fwd_func('get_fps')
    },

    _setters = {
        current_index = BoundObject.fwd_func('set_current_index'),
        fps = BoundObject.fwd_func('set_fps')
    }
})

--------------------------------------------------------------------------------

Sprite = BoundObject:extend({
    lib = dab_sprite,

-- Hook Overloads
    realize = function(class, texname, cell_size)
        local ret = class.lib.new(texname, cell_size)
        return ret
    end,

-- Bound Functions

    -- add_animation(sprite, animation, name)
    --
    -- Adds a SpriteAnimation as the given `name`
    add_animation = BoundObject.fwd_adder("add_animation"),

    -- use_animation(sprite, name)
    --
    -- Use the animation registered as `name`. If that is already the current
    -- animation, nothing happens.
    use_animation = BoundObject.fwd_func('use_animation'),

    _getters = {
        current_animation = BoundObject.fwd_func('get_current_animation'),
        direction = BoundObject.fwd_func('get_direction')
    },

    _setters = {
        current_animation = BoundObject.readonly,
        direction = BoundObject.fwd_func('set_direction')
    }
})
