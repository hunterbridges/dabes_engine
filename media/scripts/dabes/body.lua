-- Body
--
-- This BoundObject represents a Body (body.h)

require 'dabes.bound_object'

Body = BoundObject:extend({
    lib = dab_body,

-- Default Configuration

-- Hook Overloads
    realize = function(class, w, h, m, can_rotate)
        return class.lib.new(w, h, m, can_rotate)
    end,

-- Function Bindings
    apply_force = BoundObject.fwd_func("apply_force"),

    _getters = {
        pos = BoundObject.fwd_func("get_pos"),
        velo = BoundObject.fwd_func("get_velo"),
        force = BoundObject.fwd_func("get_force"),
        angle = BoundObject.fwd_func("get_angle"),
        friction = BoundObject.fwd_func("get_friction"),
        mass = BoundObject.fwd_func("get_mass"),
        can_rotate = BoundObject.fwd_func("get_can_rotate")
    },

    _setters = {
        pos = BoundObject.fwd_func("set_pos"),
        velo = BoundObject.fwd_func("set_velo"),
        force = BoundObject.fwd_func("set_force"),
        angle = BoundObject.fwd_func("set_angle"),
        friction = BoundObject.fwd_func("set_friction"),
        mass = BoundObject.fwd_func("set_mass"),
        can_rotate = BoundObject.fwd_func("set_can_rotate")
    }

})
