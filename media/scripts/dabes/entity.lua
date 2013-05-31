-- Entity
--
-- This BoundObject represents an Entity (entity.h)

require 'dabes.bound_object'

_.Entity = BoundObject:extend({
    lib = dab_entity,

-- Default Configuration

-- Hook Overloads

    realize = function(class)
        return class.lib.new()
    end,

-- Function Bindings
   _getters = {
       controller = BoundObject.fwd_func('get_controller'),
       sprite = BoundObject.fwd_func('get_sprite'),
       body = BoundObject.fwd_func('get_body'),
       alpha = BoundObject.fwd_func('get_alpha')
   },

   _setters = {
       controller = BoundObject.fwd_func('set_controller'),
       sprite = BoundObject.fwd_func('set_sprite'),
       body = BoundObject.fwd_func('set_body'),
       alpha = BoundObject.fwd_func('set_alpha')
   },

-- Hooks

    -- spawn
    --
    -- Hook called when the entity is first added to a Scene.
    -- The Scene is not guaranteed to be started.
    spawn = function(self) end,

    -- main
    --
    -- Hook called once every frame.
    --
    -- If Entity is in a physics Scene, main is called after the engine's
    -- solve step.
    main = function(self) end,

    -- presolve
    --
    -- Hook called before the physics solve step.
    --
    -- If Entity is not in a physics Scene, presolve is not called.
    presolve = function(self) end

})
Entity = _.Entity
