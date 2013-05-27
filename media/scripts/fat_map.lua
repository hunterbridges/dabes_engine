require 'dabes.scene'

FatMap = Scene:extend({
    kind = "ortho_chipmunk",
    pixels_per_meter = 32.0,

    init = function(self)
        self:load_map("media/tilemaps/fat.tmx", 2.0)

        music = Music:new(
            "media/music/Climb_Intro.ogg",
            "media/music/Climb_Loop.ogg"
        )
        self.music = music
        music:play()

        self:start()
    end,

    configure = function(self)
        -- Entities

        -- Parallax
        self.parallax = self.gen_parallax()
    end,

-- Private
    gen_parallax = function()
        yo = 80

        m_far = ParallaxLayer:new(
            "media/bgs/icecap_mountains_far.png")
        m_far.p_factor = 0.2
        m_far.offset = {0, yo - 103 - 102}

        m_close = ParallaxLayer:new(
            "media/bgs/icecap_mountains_close.png")
        m_close.p_factor = 0.2
        m_close.offset = {0, yo - 102}

        c_far = ParallaxLayer:new(
            "media/bgs/icecap_clouds_far.png")
        c_far.p_factor = 0.3
        c_far.offset = {0, yo}

        c_mid = ParallaxLayer:new(
            "media/bgs/icecap_clouds_mid.png")
        c_mid.p_factor = 0.45
        c_mid.offset = {0, yo + 25}
        c_mid.y_wiggle = 25

        c_close = ParallaxLayer:new(
            "media/bgs/icecap_clouds_close.png")
        c_close.p_factor = 0.60
        c_close.offset = {0, yo + 25 + 25}
        c_close.y_wiggle = 25 + 25

        parallax = Parallax:new(m_far, m_close, c_far,
                                c_mid, c_close)
        parallax.sky_color = {0.0,   0.0,   0.698, 1.0}
        parallax.sea_color = {0.851, 0.851, 0.851, 1.0}
        parallax.y_wiggle = -20
        parallax.sea_level = 1.0

        return parallax
    end
})

