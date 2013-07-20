--- A global object that creates an entry and transition point for
-- @{scene|Scenes}.
--
--  Subclass at your own risk.
--
--  @{scene_manager|SceneManager} extends @{object|Object}
--
-- @module scene_manager
-- @type SceneManager
require 'dabes.object'

SceneManager = Object:extend({

--- Properties.
-- Significant fields on an instance.
-- @section properties

    --- @{scene|Scene} instance the game engine is currently running.
    current_scene = nil,

    --- @{scene|Scene} instance that is being transitioned to.
    queued_scene = nil,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Enqueues a @{scene|Scene} that will be transitioned to at the
    -- end of the current frame.
    -- @name scene_manager:push_scene
    -- @tparam Scene scene The scene to transition to
    push_scene = function(self, scene)
        self.queued_scene = scene
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called at the end of a frame. This is responsible for
    -- transitioning to the `queued_scene`.
    -- @name scene_manager.flip_scene
    -- @tparam SceneManager self The `SceneManager` instance
    flip_scene = function(self)
        if self.queued_scene == nil then return end

        if self.current_scene == nil then
            self.current_scene = self.queued_scene
            self.queued_scene = nil
            self.current_scene:start()
            self.current_scene:fade_in(self.current_scene.fade_in_effect)
        else
            self.inbound_scene = self.queued_scene
            self.queued_scene = nil
            self.current_scene:fade_out(self.current_scene.fade_out_effect,
                function(scene)
                    self.current_scene:stop()
                    self.current_scene = self.inbound_scene
                    self.inbound_scene = nil
                    self.current_scene:fade_in(self.current_scene.fade_in_effect)
                    self.current_scene:start()
                end)
        end
    end,

    _noinject = true
})

--- Globals.
-- Variables that are stored in the global namespace, usually for special
-- purposes.
-- @section globals

--- The "singleton" instance of `SceneManager`.
--
-- The game engine is aware of this instance and calls the `flip_scene` hook
-- on it. Be aware that modifying `flip_scene` could have adverse effects on
-- the game's ability to function.
scene_manager = SceneManager:new()
