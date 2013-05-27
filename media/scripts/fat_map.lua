require 'dabes.scene'

FatMap = Scene:extend({
    kind = "ortho_chipmunk",
    pixels_per_meter = 32.0,

    init = function(self)
        self:load_map("media/tilemaps/fat.tmx", 2.0)
        self:start()

        -- Play music

        self.debug_camera = true
    end,

    configure = function(self)
        print "Configure hook"

        -- Entities
        -- Parallax
    end
})

