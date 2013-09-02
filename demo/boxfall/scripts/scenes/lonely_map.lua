require 'dabes.controller'
require 'dabes.scene'
require 'entities.megaman'
require 'entities.door'
require 'overlays.test_overlay'
require 'overlays.another_overlay'

LonelyMap = Scene:extend({
    kind = "chipmunk",
    pixels_per_meter = 64.0,

    init = function(self)
        self:load_map("media/tilemaps/lonely.tmx", 1.0)
        self.gravity = {0, 9.8}
    end,

    fade_in_effect = function(scene, e)
        scene.camera.scale = e.value
    end,

    fade_out_effect = function(scene, e)
        scene.camera.scale = 1.0 - e.value
    end,

    started = function(self)
        -- Entities
        local num_boxes = 15
        local xo = 6.0 / 2

        local megaman = Megaman:new()
        megaman.body.pos = {5.0 / 2, 23.25 / 2}
        megaman.controller = get_controller(1)
        megaman.z_index = 3
        self.entities:add(megaman)
        self.camera:track_entities(megaman)
        self.camera.snap_to_scene = true

        self.bg_color = {0.3, 0.3, 0.3, 1.0};

        local test_overlay = TestOverlay:new()
        test_overlay.z_index = 2
        self.overlays:add(test_overlay)

        local another_overlay = AnotherOverlay:new()
        another_overlay.z_index = 1
        self.overlays:add(another_overlay)
    end
})
