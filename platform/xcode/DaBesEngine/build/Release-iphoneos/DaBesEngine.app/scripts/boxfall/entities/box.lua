require 'dabes.entity'

Box = Entity:extend({
    init = function(self)
        local body = Body:new("chipmunk", 1.0, 1.0, 10, true)
        body.friction = 0.5
        body.elasticity = 0
        self.body = body

        self.sprite = self.build_sprite()
    end,

    build_sprite = function()
        local sprite = Sprite:new("media/sprites/dumblock.png", {32, 32}, 0)

        return sprite
    end
})
