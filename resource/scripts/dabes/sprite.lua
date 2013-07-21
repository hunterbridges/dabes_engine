--- A drawable graphic with multiple frames determined by a sprite sheet.
--
-- @{sprite|Sprite} extends @{bound_object|BoundObject}
--
-- @module sprite
-- @type Sprite

require 'dabes.bound_object'
require 'dabes.sprite_animation'

Sprite = BoundObject:extend({
    lib = dab_sprite,

-- Hook Overloads
    realize = function(class, texname, cell_size, padding)
        local ret = class.lib.new(texname, cell_size, padding)
        if ret == nil then
            error("Sprite: Texture image invalid or not found.", 3)
        end

        return ret
    end,

-- Bound Functions

    -- add_animation(sprite, animation, name)
    --
    -- Adds a SpriteAnimation as the given `name`
    add_animation = BoundObject.fwd_adder("add_animation"),

    -- get_animation(sprite, name)
    --
    -- Gets SpriteAnimation with the given `name`
    get_animation = BoundObject.fwd_func("get_animation"),

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
