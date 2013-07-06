require 'dabes.scene_manager'
require 'scenes.fat_map'
require 'scenes.reasonable_map'

function boot()
    local map = FatMap:new()
    scene_manager:push_scene(map)
end

