require 'dabes.scene_manager'
require 'scenes.fat_map'
require 'scenes.reasonable_map'
require 'scenes.lonely_map'
require 'dabes.telemetry_splash'

function boot()
    local map = ReasonableMap:new()
    local splash = TelemetrySplash:new(map)
    scene_manager:push_scene(splash)
end

