require 'dabes.controller'
require 'dabes.scene'
require 'entities.megaman'
require 'entities.door'
require 'overlays.test_overlay'

LonelyMap = Scene:extend({
    kind = "ortho_chipmunk",
    pixels_per_meter = 64.0,

    init = function(self)
        self:load_map("media/tilemaps/lonely.tmx", 1.0)
    end,

    fade_in_effect = function(scene, e)
        scene.camera.scale = e.value
    end,

    fade_out_effect = function(scene, e)
        scene.camera.scale = 1.0 - e.value
    end,

    configure = function(self)
        -- Entities
        local num_boxes = 15
        local xo = 6.0 / 2

        local megaman = Megaman:new()
        megaman.body.pos = {5.0 / 2, 23.25 / 2}
        megaman.controller = get_controller(1)
        megaman.z_index = 3
        self:add_entity(megaman)
        self.camera:track_entities(megaman)
        self.camera.snap_to_scene = true

        self.bg_color = {0.3, 0.3, 0.3, 1.0};

        local test_overlay = TestOverlay:new()
        self:add_overlay(test_overlay)
    end
})
