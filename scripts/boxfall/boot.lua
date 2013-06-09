require 'boxfall.scenes.fat_map'
require 'boxfall.scenes.reasonable_map'

function boot()
    map = FatMap:new()

    scene_manager = {
        get_current_scene = function()
            return map.real
        end
    }
end

