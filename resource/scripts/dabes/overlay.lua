require 'dabes.bound_object'

Overlay = BoundObject:extend({
    lib = dab_overlay,

-- Config

    font_name = nil,
    font_size = 0,

-- Hook Overloads

    realize = function(class)
        local realized = class.lib.new(class.font_name, class.font_size)
        if realized == nil then
            error("Overlay: Unable to open font `" .. font_name .. "`", 3)
        end

        return realized
    end,

-- Function Bindings
    draw_string = BoundObject.fwd_func_opts( "draw_string",
        {"string", "color", "origin", "align", "shadow_color", "shadow_offset"},
        {align = "left", color = {1, 1, 1, 1}, origin = {0, 0}}),

    draw_sprite = BoundObject.fwd_func_opts( "draw_sprite",
        {"sprite", "color", "center", "rotation", "scale"},
        {color = {0, 0, 0, 0}, center = {0, 0}, rotation = 0, scale = {1, 1}}),

    _getters = {
        scene = BoundObject.fwd_func("get_scene"),
        z_index = BoundObject.fwd_func("get_z_index")
    },

    _setters = {
        scene = BoundObject.readonly,
        z_index = BoundObject.fwd_func("set_z_index")
    },

-- Hooks

    -- update
    --
    -- Called once each frame during the Scene's update cycle.
    update = function(self) end,

    -- render
    --
    -- Called once each frame during the Scene's render cycle.
    render = function(self) end,

})
