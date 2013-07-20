--- An interface for modifying how a @{scene|Scene} is projected to the player.
--
-- A `Camera` is an implicit property of @{scene|Scene}. You shouldn't have
-- to create one yourself.
--
-- @{camera|Camera} extends @{bound_object|BoundObject}
--
-- @module camera
-- @type Camera

require 'dabes.bound_object'

Camera = BoundObject:extend({
    lib = dab_camera,

    realize = function(class, scene)
        return class.lib.new(scene.real)
    end,

-- Function Bindings

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {

        --- An `{x, y}` vector for the tracking center of the camera in pixels.
        -- This is set automatically in single and multiple mode.
        -- @see camera:track_entities
        focal = BoundObject.fwd_func("get_focal"),

        --- Maximum scale the camera will zoom in to.
        max_scale = BoundObject.fwd_func("get_max_scale"),

        --- Minimum scale a camera will zoom out to.
        min_scale = BoundObject.fwd_func("get_min_scale"),

        --- The angle of the `Camera` in radians.
        --
        -- A positive value rotates it clockwise, a negative value
        -- counterclockwise. Remember that this is the camera's rotation,
        -- so the @{scene|Scene} will appear to rotate in the opposite
        -- direction.
        --
        -- Default is `0`.
        rotation = BoundObject.fwd_func("get_rotation"),

        --- The zoom factor of the `Camera`.
        --
        -- 1 means 1:1 pixel ratio with the screen.
        -- Greater than 1.0 means zoom in.
        -- Less than 1.0 means zoom out.
        --
        -- Minimum is 0. Default is `1`.
        scale = BoundObject.fwd_func("get_scale"),

        --- *(read only)* An `{x, y}` vector of the screen size in pixels.
        screen_size = BoundObject.fwd_func("get_screen_size"),

        --- **(bool)** Whether the camera should stop panning at the edge of the
        -- `Scene`.
        --
        -- Default is `true`
        --
        -- **Warning:** This is ignored in multiple mode.
        snap_to_scene = BoundObject.fwd_func("get_snap_to_scene"),

        --- An `{x, y}` vector of the translation offset from `focal` in pixels.
        --
        -- This is completely independent of `focal`. For example, it could
        -- be animated with an @{easer|Easer} to create an earthquake effect.
        translation = BoundObject.fwd_func("get_translation"),
    },

    _setters = {
        focal = BoundObject.fwd_func("set_focal"),
        max_scale = BoundObject.fwd_func("set_max_scale"),
        min_scale = BoundObject.fwd_func("set_min_scale"),
        rotation = BoundObject.fwd_func("set_rotation"),
        scale = BoundObject.fwd_func("set_scale"),
        screen_size = BoundObject.readonly,
        snap_to_scene = BoundObject.fwd_func("set_snap_to_scene"),
        translation = BoundObject.fwd_func("set_translation"),
    },

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Provide a list of @{entity|Entities} for the camera to track.
    --
    -- This works in three modes:
    --
    -- `nil` - Free mode. The camera does not move unless controlled manually.
    --
    -- `Entity` - Single mode. The camera pans to keep `entity` in view.
    --
    -- `Entity`, `Entity`, ... - Multiple mode. The camera pans and zooms
    -- to keep them all in view.
    --
    -- @function camera:track_entities
    -- @param ... The @{entity|Entities} to track, or `nil`
    -- @treturn nil
    track_entities = BoundObject.fwd_func_real("track_entities"),

})
