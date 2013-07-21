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

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- *(read only)* The current `SpriteAnimation` that is playing.
        current_animation = BoundObject.fwd_func('get_current_animation'),

        --- A number representing the current direction the sprite is facing.
        -- Can be `0` (right) or `180` (left). A value of `180` will flip
        -- the sprite horizontally.
        --
        -- Default `0`
        direction = BoundObject.fwd_func('get_direction')
    },

    _setters = {
        current_animation = BoundObject.readonly,
        direction = BoundObject.fwd_func('set_direction')
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Sprite` from a sprite sheet.
    --
    -- @function Sprite:new
    -- @tparam string file_name The file name of the sprite sheet image to load.
    -- @tparam table cell_size An `{x, y}` vector representing the size of a
    -- single frame in pixels.
    -- @tparam number padding The amount of padding between each cell in pixels.
    -- @treturn Sprite
    realize = function(class, texname, cell_size, padding)
        local ret = class.lib.new(texname, cell_size, padding)
        if ret == nil then
            error("Sprite: Texture image invalid or not found.", 3)
        end

        return ret
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Adds a @{sprite_animation|SpriteAnimation} to `Sprite` with the
    -- given `name`.
    --
    -- @function sprite:add_animation
    -- @tparam SpriteAnimation animation The animation to add
    -- @tparam string name The name for the animation.
    -- @treturn nil
    add_animation = BoundObject.fwd_adder("add_animation"),

    --- Find the @{sprite_animation|SpriteAnimation} with the given `name`
    --
    -- @function sprite:get_animation
    -- @tparam string name The name of the animation to retrieve.
    -- @treturn SpriteAnimation
    get_animation = BoundObject.fwd_func("get_animation"),

    --- Use the animation named `name`. If that is already the current
    -- animation or the animation is not found, nothing happens.
    --
    -- @function sprite:use_animation
    -- @tparam string name The name of the animation to use.
    -- @treturn nil
    use_animation = BoundObject.fwd_func('use_animation'),

})
