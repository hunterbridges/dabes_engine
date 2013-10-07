--- The logical encapsulation of a single game object within a @{scene|Scene}.
--
-- `Entity` is responsible for many of the behavioral hooks that define a
-- game object.
--
-- @{entity|Entity} extends @{bound_object|BoundObject}
--
-- @module entity
-- @type Entity

require 'dabes.bound_object'

Entity = BoundObject:extend({
    lib = dab_entity,

--- Properties.
-- Significant fields on an instance.
-- @section properties

   _getters = {
       --- The opacity of the `Entity`, between `0` and `1`.
       --
       -- Default `1`
       alpha = BoundObject.fwd_func('get_alpha'),

       --- **(bool)** Whether the entity is being controlled by the system and
       -- should ignore user input.
       auto_control = BoundObject.fwd_func('get_auto_control'),

       --- The @{body|Body} that represents `Entity` in physics space.
       --
       -- Not necessary for static scenes.
       body = BoundObject.fwd_func('get_body'),

       --- An `{x, y}` vector of the Entity's center in meters.
       --
       -- If `Entity` has a `body`, this command forwards to
       -- @{body.pos|body.pos}.
       center = BoundObject.fwd_func('get_center'),

       --- The @{controller|Controller} that is controlling `Entity`
       controller = BoundObject.fwd_func('get_controller'),

       --- **(bool)** Whether to force a recorder keyframe this frame.
       force_keyframe = BoundObject.fwd_func('get_force_keyframe'),

       --- *(read only)* The @{scene|Scene} that contains `Entity`
       scene = BoundObject.fwd_func('get_scene'),

       --- An `{x, y}` vector of the Entity's size in meters.
       --
       -- This can be set in `static` Scenes. Setting this in Physics
       -- scenes will have no effect.
       size = BoundObject.fwd_func('get_size'),

       --- The @{sprite|Sprite} linked with the `Entity`
       sprite = BoundObject.fwd_func('get_sprite'),

       --- The z position of the entity. This only affects the order in which
       -- the entity appears in relation to other rendered objects. This can be
       -- between `0.0` (closest to the screen, in front) and `-256.0`
       -- (farthest from the screen, in back).
       --
       -- Default `-100.0`
       z = BoundObject.fwd_func('get_z'),
   },

   _setters = {
       alpha = BoundObject.fwd_func('set_alpha'),
       auto_control = BoundObject.fwd_func('set_auto_control'),
       body = BoundObject.fwd_func_real('set_body'),
       center = BoundObject.fwd_func('set_center'),
       controller = BoundObject.fwd_func_real('set_controller'),
       force_keyframe = BoundObject.fwd_func('set_force_keyframe'),
       scene = BoundObject.readonly,
       size = BoundObject.fwd_func('set_size'),
       sprite = BoundObject.fwd_func_real('set_sprite'),
       z = BoundObject.fwd_func('set_z'),
   },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Entity`
    --
    -- @name Entity:new
    -- @treturn Entity
    realize = function(class)
        return class.lib.new()
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when the entity is first added to a Scene.
    --
    -- The Scene may not be started.
    -- @tparam Entity self The instance
    spawn = function(self) end,

    --- Called once every frame.
    --
    -- The Scene is guaranteed to be started.
    --
    -- If Entity is in a physics Scene, main is called after the engine's
    -- solve step.
    -- @tparam Entity self The instance
    main = function(self) end,

    --- Called before the physics solve step.
    --
    -- The Scene is guaranteed to be started.
    --
    -- If Entity is not in a physics Scene, presolve is not called.
    -- @tparam Entity self The instance
    presolve = function(self) end

})
