require 'dabes.scene'

FatMap = Scene:extend({
    kind = "ortho_chipmunk",
    pixels_per_meter = 32.0,

    init = function(self)
        -- Play music

        self:load_map("media/tilemaps/fat.tmx", 2.0)

        self.debug_camera = true
    end,

    configure = function(self)
        -- Entities
        -- Parallax
    end
})

