-- SceneManager

SceneManager = Object:extend({
    current_scene = nil,
    queued_scene = nil,

    push_scene = function(self, scene)
        self.queued_scene = scene
    end,

    flip_scene = function(self)
        if self.queued_scene == nil then return end
        print(engine)

        if self.current_scene == nil then
            self.current_scene = self.queued_scene
            self.current_scene:start()
        else
            self.current_scene:stop()
            self.current_scene = self.queued_scene
            self.current_scene:start()
        end
        self.queued_scene = nil
    end,

    _noinject = true
})
scene_manager = SceneManager:new()
