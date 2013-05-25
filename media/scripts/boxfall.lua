require 'fat_map'

fat_map = FatMap:new()

scene_manager = {
    get_current_scene = function()
        return fat_map.real
    end
}

