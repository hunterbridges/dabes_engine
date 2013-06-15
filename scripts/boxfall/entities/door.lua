require 'dabes.entity'
require 'dabes.sensor'
inspect = require 'lib.inspect'

Door = Entity:extend({
    body_type = "chipmunk",

    init = function(self)
        local w = 1.0
        local h = 2.0
        local body = Body:new(self.body_type, w, h, 1, false)

        local hbw = 1.0
        local hbh = 56 / 64
        body:set_hit_box(hbw, hbh, {0.0, 8 / 64})
        body.is_static = true
        self.body = body

        self.sprite = self.build_sprite()
        self.alpha = 0

        local open_sensor = Sensor:new(0.1, 0.1,
                                       {0, hbh / 2 * h - 0.04})
        self.open_sensor = open_sensor
        body:add_sensor(open_sensor)
    end,

    build_sprite = function()
        local sprite = Sprite:new("media/sprites/door.png", {32, 64}, 1)

        local closed = SpriteAnimation:new(0)
        closed.fps = 0
        sprite:add_animation(closed, "closed")

        local opening = SpriteAnimation:new(0, 1, 2, 3)
        opening.fps = 4
        opening.repeats = false
        sprite:add_animation(opening, "opening")

        sprite:use_animation("closed")
        return sprite
    end,

    open = function(self)
        local opening = self.sprite:get_animation("opening")
        opening.complete = function(anim)
            local scene = self.destination:new()
            scene_manager:push_scene(scene)
            opening.complete = function() end
        end
        self.sprite:use_animation("opening")
    end
})
