require 'dabes.scene_manager'
require 'scenes.fat_map'
require 'scenes.reasonable_map'
require 'scenes.lonely_map'

function boot()
    local map = LonelyMap:new()
    scene_manager:push_scene(map)
end

