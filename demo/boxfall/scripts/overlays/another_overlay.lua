require 'dabes.overlay'
require 'dabes.sprite'

AnotherOverlay = Overlay:extend({
    init = function(self)
   		self.timer = 0

        self.asprite = Sprite:new("media/sprites/dumblock.png", {32, 32}, 0)
    end,

    render = function(self)
        local screen_size = self.scene.camera.screen_size
        local o = {0, 0}
        o[1] = o[1] - screen_size[1] / 2 + 20
        o[2] = o[2] - screen_size[2] / 2 + 14

        o[1] = 0
        self:draw_sprite({
        	sprite = self.asprite.real,
        	center = o,
        	scale = 5
        })
    end
})
