require 'dabes.overlay'
require 'dabes.sprite'

TestOverlay = Overlay:extend({
    font_name = "media/fonts/uni.ttf",
    font_size = 16,
    
    init = function(self)
   		self.timer = 0

        self.asprite = Sprite:new("media/sprites/dumblock.png", {32, 32}, 0)
    end,

    update = function(self)
        local timer = math.floor((dab_engine.ticks() - self.born_at)/1000)
        self.timer = timer
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
        
        o[2] = o[2] + 4
        
        self:draw_string({
            string = ""..self.timer,
            color = {1, 1, 1, 1},
            origin = o,
            shadow_color = {0, 0, 0, 0.5},
            shadow_offset = {2, 2},
            align = "center"
        })
        
        o[2] = o[2] + 36
        
        self:draw_string({
            string = "\"THE TIMER\"",
            color = {1, 1, 1, 1},
            origin = o,
            shadow_color = {0, 0, 0, 0.5},
            shadow_offset = {2, 2},
            align = "center"
        })
        
        o[1] = -screen_size[1] / 2 + 10
        o[2] = screen_size[2] / 2 - 10
        
        self:draw_string({
            string = "LIVES x 1",
            color = {1, 1, 1, 1},
            origin = o,
            shadow_color = {0, 0, 0, 0.5},
            shadow_offset = {2, 2},
            align = "left"
        })
    end
})
