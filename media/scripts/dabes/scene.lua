-- Scene
--
-- This BoundObject represents a Scene (scene.h)

require 'dabes.bound_object'
require 'dabes.music'
require 'dabes.parallax'

Scene = BoundObject:extend({
    lib = dab_scene,

-- Default Configuration

    kind = "static",
    pixels_per_meter = 32.0,

-- Hook Overloads

    realize = function(class)
        return class.lib.new(class.kind, class.pixels_per_meter)
    end,

-- Function Bindings

    -- start(self)
    --
    -- Starts the scene. Make sure you do this in init()
    start = BoundObject.fwd_func("start"),

    -- load_map(self, map, meters_per_tile)
    --
    -- Loads the given `map` .tmx resource, represented in the game world
    -- at the size `meters_per_tile`
    load_map = BoundObject.fwd_func("load_map"),

    -- add_entity(self, entity)
    --
    -- Adds an Entity to the Scene.
    add_entity = BoundObject.fwd_func("add_entity"),

    _getters = {
        space = BoundObject.fwd_func("get_space"),
        music = BoundObject.fwd_func("get_music"),
        parallax = BoundObject.fwd_func("get_parallax"),
        draw_grid = BoundObject.fwd_func("get_draw_grid"),
        debug_camera = BoundObject.fwd_func("get_debug_camera")
    },

    _setters = {
        space = BoundObject.readonly,
        music = BoundObject.fwd_func("set_music"),
        parallax = BoundObject.fwd_func("set_parallax"),
        draw_grid = BoundObject.fwd_func("set_draw_grid"),
        debug_camera = BoundObject.fwd_func("set_debug_camera")
    },

-- Hooks

    -- configure
    --
    -- Hook called by the game engine when the scene starts. Overload this to
    -- do things like add entities and parallax layers.
    --
    -- Configure may be called multiple times in the Scene's lifecycle.
    -- If you want to do things that persist across restarts,
    -- do them in `init` (inherited from BoundObject)
    configure = function(self) end

})
