-- Camera
--
-- This BoundObject represents a Camera (camera.h)

require 'dabes.bound_object'

Camera = BoundObject:extend({
    lib = dab_camera,

-- Hook Overloads

    realize = function(class, scene)
        return class.lib.new(scene.real)
    end,

-- Function Bindings

    track_entities = BoundObject.fwd_func_real("track_entities"),

    _getters = {
        snap_to_scene = BoundObject.fwd_func("get_snap_to_scene"),
        max_scale = BoundObject.fwd_func("get_max_scale"),
        min_scale = BoundObject.fwd_func("get_min_scale"),
        scale = BoundObject.fwd_func("get_scale"),
        rotation = BoundObject.fwd_func("get_rotation"),
        translation = BoundObject.fwd_func("get_translation"),
        focal = BoundObject.fwd_func("get_focal"),
        screen_size = BoundObject.fwd_func("get_screen_size")
    },

    _setters = {
        snap_to_scene = BoundObject.fwd_func("set_snap_to_scene"),
        max_scale = BoundObject.fwd_func("set_max_scale"),
        min_scale = BoundObject.fwd_func("set_min_scale"),
        scale = BoundObject.fwd_func("set_scale"),
        rotation = BoundObject.fwd_func("set_rotation"),
        translation = BoundObject.fwd_func("set_translation"),
        focal = BoundObject.fwd_func("set_focal"),
        screen_size = BoundObject.readonly
    }
})
