-- Sprite
--
-- This BoundObject represents a Sprite (sprite.h)

require 'dabes.bound_object'

SpriteAnimation = BoundObject.extend({
    lib = dabes_spriteanimation,

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

Sprite = BoundObject.extend({
    lib = dabes_sprite,

-- Hook Overloads
    realize = function(class, cell_size)
        return class.lib.new(cell_size)
    end

-- Bound Functions
    add_animation = BoundObject.fwd_func('add_animation'),

    use_animation = BoundObject.fwd_func('use_animation'),

    _getters = {
        current_animation = BoundObject.fwd_func('get_current_animation'),
        direction = BoundObject.fwd_func('get_direction'),
    },

    _setters = {
        current_animation = BoundObject.readonly,
        direction = BoundObject.fwd_func('set_direction')
    }

})
