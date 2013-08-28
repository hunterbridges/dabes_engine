require 'dabes.scene'
require 'dabes.scene_manager'

TelemetrySplash = Scene:extend({
    kind = "telemetry",

    init = function(self, next_scene)
        self.next_scene = next_scene
    end,

    finish_splash = function(self)
        if self.next_scene ~= nil then
            scene_manager:push_scene(self.next_scene)
        end
    end
})
