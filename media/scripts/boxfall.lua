require 'fat_map'

fat_map = Scene:new(FatMap)

scene_manager = {
    get_current_scene = function()
        return fat_map.real
    end
}

