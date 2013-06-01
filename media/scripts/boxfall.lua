require 'boxfall.scenes.fat_map'

function boot()
    local fat_map = retain(FatMap:new())

    scene_manager = {
        get_current_scene = function()
            return fat_map.real
        end
    }
end

