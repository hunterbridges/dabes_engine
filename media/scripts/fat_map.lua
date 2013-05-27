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
        print "Configure hook"

        -- Entities
        -- Parallax
    end
})

