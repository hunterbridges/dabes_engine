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
    draw_string = BoundObject.fwd_func("draw_string"),

    _getters = {
        scene = BoundObject.fwd_func("get_scene")
    },

    _setters = {
        scene = BoundObject.readonly
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
