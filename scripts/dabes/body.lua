-- Body
--
-- This BoundObject represents a Body (body.h)

require 'dabes.bound_object'

Body = BoundObject:extend({
    lib = dab_body,

-- Default Configuration

-- Hook Overloads
    realize = function(class, proto, w, h, m, can_rotate)
        return class.lib.new(proto, w, h, m, can_rotate)
    end,

-- Function Bindings

    -- apply_force(body, {x, y} force, {x, y} offset)
    --
    -- Apply `force` Newtons `offset` meters from the center of body.
    apply_force = BoundObject.fwd_func("apply_force"),

    -- set_hit_box(body, w, h, {x, y} offset)
    --
    -- OK This is a weird function.
    --
    -- Make a hitbox of the relative size w, h, and set the physical center
    -- `offset` meters from the draw center. It is important to note that
    -- the offset is applied and becomes the body's center.
    set_hit_box = BoundObject.fwd_func("set_hit_box"),

    -- add_sensor(body, sensor)
    --
    -- Adds a sensor to body.
    add_sensor = BoundObject.fwd_adder("add_sensor"),

    -- remove_sensor(body, sensor)
    --
    -- Removes a sensor from body.
    remove_sensor = BoundObject.fwd_remover("remove_sensor"),

    _getters = {
        entity = BoundObject.fwd_func("get_entity"),
        pos = BoundObject.fwd_func("get_pos"),
        velo = BoundObject.fwd_func("get_velo"),
        force = BoundObject.fwd_func("get_force"),
        angle = BoundObject.fwd_func("get_angle"),
        friction = BoundObject.fwd_func("get_friction"),
        mass = BoundObject.fwd_func("get_mass"),
        can_rotate = BoundObject.fwd_func("get_can_rotate"),
        is_rogue = BoundObject.fwd_func("get_is_rogue"),
        is_static = BoundObject.fwd_func("get_is_static")
    },

    _setters = {
        entity = BoundObject.readonly,
        pos = BoundObject.fwd_func("set_pos"),
        velo = BoundObject.fwd_func("set_velo"),
        force = BoundObject.fwd_func("set_force"),
        angle = BoundObject.fwd_func("set_angle"),
        friction = BoundObject.fwd_func("set_friction"),
        mass = BoundObject.fwd_func("set_mass"),
        can_rotate = BoundObject.fwd_func("set_can_rotate"),
        is_rogue = BoundObject.fwd_func("set_is_rogue"),
        is_static = BoundObject.fwd_func("set_is_static")
    }
})
