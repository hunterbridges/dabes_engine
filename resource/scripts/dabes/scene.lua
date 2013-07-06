-- Scene
--
-- This BoundObject represents a Scene (scene.h)

require 'dabes.bound_object'
require 'dabes.camera'
require 'dabes.easer'
require 'dabes.music'
require 'dabes.parallax'

Scene = BoundObject:extend({
    lib = dab_scene,

-- Default Configuration

    kind = "static",
    pixels_per_meter = 32.0,

-- Hook Overloads

    realize = function(class)
        if class.kind ~= 'ortho_chipmunk' then
            error("Invalid class kind `" .. class.kind .. "`")
        end
       
        return class.lib.new(class.kind, class.pixels_per_meter)
    end,

-- Functions
    fade_in = function(scene, more_update, completion)
        if scene._fade_in_easer ~= nil then return end

        local easer = Easer:new(500)
        easer.update = function(e)
            local color = scene.cover_color
            color[4] = 1.0 - e.value
            scene.cover_color = color
            if more_update ~= nil then more_update(scene, e) end
        end
        easer.finish = function(e)
            if completion ~= nil then completion(scene) end
            scene._fade_in_easer = nil
        end
        scene._fade_in_easer = easer
    end,

    fade_out = function(scene, more_update, completion)
        if scene._fade_out_easer ~= nil then return end

        local easer = Easer:new(500)
        easer.update = function(e)
            local color = scene.cover_color
            color[4] = e.value
            scene.cover_color = color
            if more_update ~= nil then more_update(scene, e) end
        end
        easer.finish = function(e)
            if completion ~= nil then completion(scene) end
            scene._fade_out_easer = nil
        end
        scene._fade_out_easer = easer
    end,

-- Function Bindings

    -- start(self)
    --
    -- Starts the scene.
    start = BoundObject.fwd_func("start"),

    -- stop(self)
    --
    -- Stops the scene.
    stop = BoundObject.fwd_func("stop"),

    -- load_map(self, map, meters_per_tile)
    --
    -- Loads the given `map` .tmx resource, represented in the game world
    -- at the size `meters_per_tile`
    load_map = BoundObject.fwd_func("load_map"),

    -- add_entity(self, entity)
    --
    -- Adds an Entity to the Scene.
    add_entity = BoundObject.fwd_adder("add_entity"),

    _getters = {
        space = BoundObject.fwd_func("get_space"),
        music = BoundObject.fwd_func("get_music"),
        parallax = BoundObject.fwd_func("get_parallax"),

        camera = function(self)
            local raw_getter = BoundObject.fwd_func("get_camera")
            local got = raw_getter(self)
            if got == nil then
                got = Camera:new(self)
                self._cache.camera = got
            end
            return got
        end,

        draw_grid = BoundObject.fwd_func("get_draw_grid"),
        debug_camera = BoundObject.fwd_func("get_debug_camera"),
        cover_color = BoundObject.fwd_func("get_cover_color")
    },

    _setters = {
        space = BoundObject.readonly,
        music = BoundObject.fwd_func("set_music"),
        parallax = BoundObject.fwd_func("set_parallax"),
        draw_grid = BoundObject.fwd_func("set_draw_grid"),
        debug_camera = BoundObject.fwd_func("set_debug_camera"),
        cover_color = BoundObject.fwd_func("set_cover_color")
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
    configure = function(self) end,

    -- cleanup
    --
    -- Hook called by the game engine when the scene stops. Overload this if you
    -- need to clean up more things, but don't forget to call back up to this
    -- method to make sure your entity cache gets cleaned up.
    cleanup = function(self)
        self._add_entity_cache = nil
        self:_cleancache()
        collectgarbage("collect")
    end,

    -- reset_camera
    --
    -- Hook called by the game engine when the scene needs to reset its camera.
    reset_camera = function(self)
        local camera = self.camera
        camera.scale = 1
        camera.rotation = 0
        camera.translation = {0, 0}
        camera.snap_to_scene = true
        camera.max_scale = 1
    end

})
