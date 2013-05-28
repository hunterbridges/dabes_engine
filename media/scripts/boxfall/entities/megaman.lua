require 'dabes.entity'

Megaman = Entity:extend({
    init = function(self)
        body = Body::new(2.0, 2.0, 100, false)
        body.friction = 0.7
        self.body = body

        self.sprite = self.build_sprite()
    end,

    build_sprite = function()
        sprite = Sprite:new("media/sprites/megaman_run.png", {32, 32})

        standing = SpriteAnimation:new("standing", 0)
        standing.fps = 0
        sprite:add_animation(standing)

        running = SpriteAnimation:new("running", 1, 2, 3, 2)
        running.fps = 7
        sprite:add_animation(running)

        sprite:use_animation("standing")

        return sprite
    end
})
