require 'dabes.scene'

FatMap = {
    i_params = {
        kind = "ortho_chipmunk",
        pixels_per_meter = 32.0
    },

    init = function(scene)
        print("Hey, here is the scene dude")
        scene.real:load_map("media/tilemaps/fat.tmx", 2.0)
    end
}

