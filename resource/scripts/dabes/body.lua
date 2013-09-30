--- A `Body` is the representation of an @{entity|Entity} in physics space.
--
-- As such, it is only relevant in @{scene|Scenes} driven by physics engines.
--
-- The `Body` is used to represent the `Entity` in the physical space.
-- The @{sprite|Sprite} represents it in the graphical space. The `Body`
-- dimensions, rotation, and position determine how the `Sprite` is drawn.
--
-- If you need to set a collision area that is smaller than the `Sprite`'s
-- cell size, use `set_hit_box`.
--
--  @{body|Body} extends @{bound_object|BoundObject}

require 'dabes.bound_object'

Body = BoundObject:extend({
    lib = dab_body,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- The current rotation of the `Body` in **radians**
        angle = BoundObject.fwd_func("get_angle"),

        --- **(bool)** Whether the `Body` can rotate on its own.
        can_rotate = BoundObject.fwd_func("get_can_rotate"),

        --- Elasticity factor, on a scale of 0.0 (Not bouncy) to 1.0 (Perfectly
        -- bouncy).
        elasticity = BoundObject.fwd_func("get_elasticity"),

        --- *(read only)* The @{entity|Entity} the `Body` is attached to.
        entity = BoundObject.fwd_func("get_entity"),

        --- The current amount of force being applied to `Body` in Newtons.
        force = BoundObject.fwd_func("get_force"),

        --- Coefficient of friction, on a scale of 0.0 (Smooth) to 1.0 (Sticky).
        friction = BoundObject.fwd_func("get_friction"),

        --- **(bool)** If `true`, then `Body` is not modified by the physics
        --solver.
        --
        -- This could be useful for something like a gate or moving platform.
        is_rogue = BoundObject.fwd_func("get_is_rogue"),

        --- **(bool)** If `true`, then `Body` does not collide with other
        -- objects. @{sensor|Sensors} attached to `Body` will still work.
        --
        -- This could be useful for something like a door or switch.
        is_static = BoundObject.fwd_func("get_is_static"),

        --- Mass in kg
        mass = BoundObject.fwd_func("get_mass"),

        --- An `{x, y}` vector of the `Body`'s center in the physics space *(in
        -- meters)*.
        --
        -- **Warning:** You must set the property explicitly for changes to take
        -- apply. Like this:
        --      body.pos = {10, 11}
        -- **Not** this:
        --      body.pos[1] = 10
        pos = BoundObject.fwd_func("get_pos"),

        --- An `{x, y}` vector of the `Body`'s velocity *(in meters per
        -- second)*.
        --
        -- **Warning:** You must set the property explicitly for changes to take
        -- apply. Like this:
        --      body.velo = {5, 0}
        -- **Not** this:
        --      body.velo[1] = 5
        velo = BoundObject.fwd_func("get_velo"),

        --- Set the collision bit-plane of the `Body`.
        --
        -- @{body|Bodies} only collide if they are in the same bit-planes.
        -- i.e. (a.collision\_layers & b.collision\_layers) != 0.
        --
        -- By default, a `Body` occupies all bit-planes.
        collision_layers = BoundObject.fwd_func("get_collision_layers"),
    },

    _setters = {
        angle = BoundObject.fwd_func("set_angle"),
        can_rotate = BoundObject.fwd_func("set_can_rotate"),
        elasticity = BoundObject.fwd_func("set_elasticity"),
        entity = BoundObject.readonly,
        force = BoundObject.fwd_func("set_force"),
        friction = BoundObject.fwd_func("set_friction"),
        is_rogue = BoundObject.fwd_func("set_is_rogue"),
        is_static = BoundObject.fwd_func("set_is_static"),
        mass = BoundObject.fwd_func("set_mass"),
        pos = BoundObject.fwd_func("set_pos"),
        velo = BoundObject.fwd_func("set_velo"),
        collision_layers = BoundObject.fwd_func("set_collision_layers"),
    },

    _collections = {
        --- The @{collection|Collection} of the Body's @{sensor|Sensors}
        -- @class field
        -- @name sensors
        sensors = {
            adder = BoundObject.fwd_func("add_sensor"),
            remover = BoundObject.fwd_func("remove_sensor")
        }
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Body`
    --
    -- @name Body:new
    -- @tparam string proto The `Body` prototype. Currently, the only valid
    -- @tparam number width The width of the body in meters.
    -- @tparam number height The height of the body in meters.
    -- @tparam number mass The mass of the body in kg.
    -- @tparam boolean can_rotate Whether the `Body` can rotate or not
    -- option is `"chipmunk"`
    -- @treturn Body
    realize = function(class, proto, w, h, m, can_rotate)
        return class.lib.new(proto, w, h, m, can_rotate)
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Apply a force on the next solve.
    --
    -- @function body:apply_force
    -- @tparam table force An `{x, y}` vector of the force in Newtons
    -- (kg * m / s ^ 2).
    --
    -- @tparam table offset An `{x, y}` vector of the offset from center
    -- the force is applied to (in respect to rotation).
    --
    -- @treturn nil
    -- @usage body:apply_force({100, 0}, {0, 0}) -- Push right
    apply_force = BoundObject.fwd_func("apply_force"),

    --- Set the `Body`'s collision box in relation to the `Body`'s dimensions.
    --
    -- @function body:set_hit_box
    -- @tparam number width The width of the hitbox, as a coefficient of the
    -- `Body`'s width.
    --
    -- @tparam number height The height of the hitbox, as a coefficient of the
    -- `Body`'s height.
    --
    -- @tparam table offset An `{x, y}` vector of the hitbox center in relation
    -- to the `Body`'s center in meters.
    --
    -- @treturn nil
    -- @usage body:set_hit_box(0.5, 0.5, {0, 10}) -- A hitbox half the size
    set_hit_box = BoundObject.fwd_func("set_hit_box"),

})
