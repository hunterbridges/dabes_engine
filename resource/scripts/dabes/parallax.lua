require 'dabes.bound_object'

ParallaxLayer = BoundObject:extend({
    lib = dab_parallaxlayer,

-- Hook Overloads

    realize = function(class, texture)
        return class.lib.new(texture)
    end,

-- Function Bindings
    _getters = {
        p_factor = BoundObject.fwd_func("get_p_factor"),
        offset = BoundObject.fwd_func("get_offset"),
        y_wiggle = BoundObject.fwd_func("get_y_wiggle")
    },

    _setters = {
        p_factor = BoundObject.fwd_func("set_p_factor"),
        offset = BoundObject.fwd_func("set_offset"),
        y_wiggle = BoundObject.fwd_func("set_y_wiggle")
    }
})

--------------------------------------------------------------------------------

Parallax = BoundObject:extend({
    lib = dab_parallax,

-- Hook Overloads

    realize = function(class, ...)
        return class.lib.new(map_real(...))
    end,

-- Public
    add_layers = function(self, ...)
        for i = 1, select("#", ...) do
            self:add_layer(select(i, ...))
        end
    end,

-- Function Bindings

    -- add_layer(parallax, layer)
    --
    -- Adds a layer on top of the existing layers.
    add_layer = BoundObject.fwd_adder("add_layer"),

    _getters = {
        sky_color = BoundObject.fwd_func("get_sky_color"),
        sea_color = BoundObject.fwd_func("get_sea_color"),
        y_wiggle = BoundObject.fwd_func("get_y_wiggle"),
        sea_level = BoundObject.fwd_func("get_sea_level")
    },

    _setters = {
        sky_color = BoundObject.fwd_func("set_sky_color"),
        sea_color = BoundObject.fwd_func("set_sea_color"),
        y_wiggle = BoundObject.fwd_func("set_y_wiggle"),
        sea_level = BoundObject.fwd_func("set_sea_level")
    }
})
