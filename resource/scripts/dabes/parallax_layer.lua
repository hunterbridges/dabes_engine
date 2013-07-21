--- A single layer of a @{parallax|Parallax}.
--
-- @{parallax|ParallaxLayer} extends @{bound_object|BoundObject}
--
-- @module parallax_layer
-- @type ParallaxLayer

require 'dabes.bound_object'

ParallaxLayer = BoundObject:extend({
    lib = dab_parallaxlayer,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- The proportion by which to scroll the `x` axis against
        -- the @{camera.focal|camera.focal} `x` position.
        --
        -- A positive `p_factor` means that `ParallaxLayer` will scroll
        -- opposite the `Camera` (camera goes right, layer goes left).
        -- If abs(`p_factor`) < `1`, it will scroll slower than the
        -- camera. Usually a positive `p_factor` between `0` and `1` is
        -- what you want.
        p_factor = BoundObject.fwd_func("get_p_factor"),

        --- An `{x, y}` vector of the layer's offset in pixels.
        --
        -- `{0, 0}` aligns the top edge of the layer with the
        -- @{parallax.sea_level|sea_level}. The `y` offset is arguably
        -- more useful than `x`. The `x` value simply changes the phase of
        -- the `x` repeat.
        --
        -- **Warning:** You must set the property explicitly for changes to take
        -- apply. Like this:
        --      layer.offset = {10, 11}
        -- **Not** this:
        --      layer.offset[2] = 11
        offset = BoundObject.fwd_func("get_offset"),

        --- The pixel range across which to translate the `ParallaxLayer`
        -- against the @{camera.focal|camera.focal} `y` position.
        --
        -- A positive `y_wiggle` means that `ParallaxLayer` will translate
        -- opposite the `Camera` (camera goes down, parallax goes up).
        -- Can be positive or negative.
        --
        -- The parent @{parallax.y_wiggle|parallax.y_wiggle} combines with this
        -- one.
        --
        -- Default `0`
        y_wiggle = BoundObject.fwd_func("get_y_wiggle")
    },

    _setters = {
        p_factor = BoundObject.fwd_func("set_p_factor"),
        offset = BoundObject.fwd_func("set_offset"),
        y_wiggle = BoundObject.fwd_func("set_y_wiggle")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `ParallaxLayer` by loading an image.
    --
    -- @function ParallaxLayer:new
    -- @tparam string image The file name of the image to load.
    -- @treturn ParallaxLayer
    realize = function(class, texture)
        local realized = class.lib.new(texture)
        if realized == nil then
            error("ParallaxLayer: Texture image invalid or not found.", 3)
        end

        return realized
    end

})

