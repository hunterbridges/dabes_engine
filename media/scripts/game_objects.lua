require "engine_objects"

function EntityConfig.create_box()
    return EntityConfig.create()
end

function EntityConfig.create_megaman()
    local entity = EntityConfig.create()

    entity.w = 2.0
    entity.h = 2.0
    entity.mass = 100
    entity.edge_friction = 0.7
    entity.can_rotate = false

    entity.sprite = {
        texture = "media/sprites/megaman_run.png",
        cell_size = {w = 32, h = 32},
        start_animation = "standing",
        num_animations = 2,
        animations = {
            {
                name =  "standing",
                num_frames = 1,
                frames = {0},
                fps = 0
            },
            {
                name = "running",
                num_frames = 4,
                frames = {1, 2, 3, 2},
                fps = 7
            }
        }
    }

    return entity
end
