require 'boxfall.scenes.fat_map'

scene_manager = {}

function boot()
    fat_map = FatMap:new()

    scene_manager = {
        get_current_scene = function()
            return fat_map.real
        end
    }
end

