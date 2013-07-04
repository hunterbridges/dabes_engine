-- SceneManager

SceneManager = Object:extend({
    current_scene = nil,
    queued_scene = nil,

    push_scene = function(self, scene)
        self.queued_scene = scene
    end,

    flip_scene = function(self)
        if self.queued_scene == nil then return end

        if self.current_scene == nil then
            self.current_scene = self.queued_scene
            self.queued_scene = nil
            self.current_scene:start()
            self.current_scene:fade_in(self.current_scene.fade_in_effect)
        else
            self.inbound_scene = self.queued_scene
            self.queued_scene = nil
            self.current_scene:fade_out(self.current_scene.fade_out_effect,
                function(scene)
                    self.current_scene:stop()
                    self.current_scene = self.inbound_scene
                    self.inbound_scene = nil
                    self.current_scene:fade_in(self.current_scene.fade_in_effect)
                    self.current_scene:start()
                end)
        end
    end,

    _noinject = true
})
scene_manager = SceneManager:new()
