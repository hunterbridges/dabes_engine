require 'dabes.controller'
require 'dabes.scene'
require 'entities.squiggy_box'
require 'entities.megaman'
require 'entities.door'
require 'scenes.fat_map'

ReasonableMap = Scene:extend({
    kind = "ortho_chipmunk",
    pixels_per_meter = 64.0,
    music_volume = 0.3,

    init = function(self)
        self:load_map("media/tilemaps/reasonable.tmx", 1.0)
    end,

    fade_in_effect = function(scene, e)
        scene.camera.scale = 2.0 - e.value
    end,

    fade_out_effect = function(scene, e)
        scene.camera.scale = 1.0 + e.value
        scene.music.volume = (1.0 - e.value) * scene.music_volume
    end,

    configure = function(self)
        -- Music
        local music = Music:new(
            "media/music/Sneak.ogg"
        )
        self.music = music
        music.volume = self.music_volume
        music:play()

        -- Entities
        local num_boxes = 15
        local xo = 6.0 / 2

        local entities = {}
        local megaman = Megaman:new()
        megaman.body.pos = {5.0 / 2, 23.25 / 2}
        megaman.controller = get_controller(1)
        megaman.z_index = 3
        self:add_entity(megaman)
        table.insert(entities, megaman)

        for i = 1, num_boxes do
            local box = SquiggyBox:new()

            local body = box.body
            body.pos = {xo, 10.0 - 4.0 - i / 3}
            body.angle = math.pi / 16.0 * (i % 8)
            body.mass = 100.0 + 900.0 * i / num_boxes

            box.alpha = 0
            box.z_index = 2
            self:add_entity(box)
            table.insert(entities, box)

            xo = xo + 1
        end

        self.camera:track_entities(megaman)
        self.camera.snap_to_scene = true

        local door = Door:new()
        door.destination = FatMap
        door.body.pos = {2.5, 11.1 + 1 / 64}
        door.z_index = 1
        self:add_entity(door)

        -- Parallax
        self.parallax = self.gen_parallax()
    end,

-- Private
    gen_parallax = function()
        local yo = 80

        local m_far = ParallaxLayer:new(
            "media/bgs/icecap_mountains_far.png")
        m_far.p_factor = 0.2
        m_far.offset = {0, yo - 103 - 102}

        local m_close = ParallaxLayer:new(
            "media/bgs/icecap_mountains_close.png")
        m_close.p_factor = 0.2
        m_close.offset = {0, yo - 102}

        local c_far = ParallaxLayer:new(
            "media/bgs/icecap_clouds_far.png")
        c_far.p_factor = 0.3
        c_far.offset = {0, yo}

        local c_mid = ParallaxLayer:new(
            "media/bgs/icecap_clouds_mid.png")
        c_mid.p_factor = 0.45
        c_mid.offset = {0, yo + 25}
        c_mid.y_wiggle = 25

        local c_close = ParallaxLayer:new(
            "media/bgs/icecap_clouds_close.png")
        c_close.p_factor = 0.60
        c_close.offset = {0, yo + 25 + 25}
        c_close.y_wiggle = 25 + 25

        local parallax = Parallax:new()
        parallax.sky_color = {0.0,   0.0,   0.698, 1.0}
        parallax.sea_color = {0.851, 0.851, 0.851, 1.0}
        parallax.y_wiggle = -20
        parallax.sea_level = 1.0
        parallax:add_layers(m_far, m_close, c_far, c_mid, c_close)

        return parallax
    end
})
