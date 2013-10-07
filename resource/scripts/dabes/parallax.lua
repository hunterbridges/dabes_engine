--- A repeating background that features proportional scrolling in
-- relation to a @{scene|Scene}'s @{camera|Camera}.
--
-- `Parallax` repeats @{parallax_layer|ParallaxLayers} through the `x` axis.
-- When the @{camera|Camera} pans along the `x` axis, the layers scroll
-- proportionately according to @{parallax_layer.p_factor|p_factor}.
-- Stacking layers back-to-front with an increasing `p_factor` can create
-- the illusion of 3D depth in 2D.
--
-- `Parallax` also offers the ability to react to `y` axis panning. Using
-- `y_wiggle` and `sea_level`, you can create the illusion of altitude.
-- Layers do not repeat through the `y` axis.
--
-- @{parallax|Parallax} extends @{bound_object|BoundObject}
--
-- @module parallax
-- @type Parallax

require 'dabes.bound_object'
require 'dabes.parallax_layer'

Parallax = BoundObject:extend({
    lib = dab_parallax,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- An `{r, g, b, a}` vector that represents the background color
        -- for the top of the `Parallax`.
        --
        -- The height of the "sky" is (screen height * `sea_level`)
        --
        -- Default `{0, 0, 1, 1}` (solid blue)
        --
        -- **Warning:** You must set the property explicitly for changes to take
        -- apply. Like this:
        --      parallax.sky_color = {0.5, 0, 0, 1}
        -- **Not** this:
        --      parallax.sky_color[1] = 0.5
        sky_color = BoundObject.fwd_func("get_sky_color"),

        --- An `{r, g, b, a}` vector that represents the background color
        -- for the bottom of the `Parallax`.
        --
        -- The height of the "sea" is (screen height * (1 - `sea_level`))
        --
        -- Default `{0, 1, 0, 1}` (solid green)
        --
        -- **Warning:** You must set the property explicitly for changes to take
        -- apply. Like this:
        --      parallax.sea_color = {0.5, 0, 0, 1}
        -- **Not** this:
        --      parallax.sea_color[1] = 0.5
        sea_color = BoundObject.fwd_func("get_sea_color"),

        --- The pixel range across which to translate the `Parallax` against
        -- the @{camera.focal|camera.focal} `y` position.
        --
        -- A positive `y_wiggle` means that `Parallax` will translate
        -- opposite the `Camera` (camera goes down, parallax goes up).
        -- Can be positive or negative.
        --
        -- Default `0`
        y_wiggle = BoundObject.fwd_func("get_y_wiggle"),

        --- The percentage across the `Scene` `y` axis where the `Parallax`
        -- considered to be "flat" when simulating altitude effects. This
        -- might be `0.0` if you are creating overhead clouds, or `1.0` if you
        -- are creating an ocean.
        --
        -- Default `0.5`
        sea_level = BoundObject.fwd_func("get_sea_level"),

        --- The z value for the `Parallax`'s sky and sea background rectangles.
        -- Default `-200.0`
        bg_z = BoundObject.fwd_func("get_bg_z")
    },

    _setters = {
        sky_color = BoundObject.fwd_func("set_sky_color"),
        sea_color = BoundObject.fwd_func("set_sea_color"),
        y_wiggle = BoundObject.fwd_func("set_y_wiggle"),
        sea_level = BoundObject.fwd_func("set_sea_level"),
        bg_z = BoundObject.fwd_func("set_bg_z")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Parallax`
    --
    -- @function Parallax:new
    -- @tparam ParallaxLayer ... A variable list of
    -- @{parallax_layer|ParallaxLayers} to add upon creation.
    -- @treturn Parallax
    realize = function(class, ...)
        return class.lib.new(map_real(...))
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Add a @{parallax_layer|ParallaxLayer} to `Parallax`
    --
    -- @function parallax:add_layer
    -- @tparam ParallaxLayer layer The layer to add
    -- @treturn nil
    add_layer = BoundObject.fwd_adder("add_layer"),

    --- Add a variable list of @{parallax_layer|ParallaxLayers} to `Parallax`
    --
    -- @function parallax:add_layers
    -- @tparam ParallaxLayer ... The layers to add
    -- @treturn nil
    add_layers = function(self, ...)
        for i = 1, select("#", ...) do
            self:add_layer(select(i, ...))
        end
    end

})
