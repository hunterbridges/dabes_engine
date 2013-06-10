require 'boxfall.scenes.fat_map'
require 'boxfall.scenes.reasonable_map'
require 'dabes.scene_manager'

function boot()
    local map = ReasonableMap:new()
    scene_manager:push_scene(map)
end

