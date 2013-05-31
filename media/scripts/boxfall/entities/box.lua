require 'dabes.entity'

_.Box = Entity:extend({
    init = function(self)
        body = Body:new("chipmunk", 1.0, 1.0, 10, true)
        body.friction = 0.5
        self.body = body

        self.sprite = self.build_sprite()
    end,

    build_sprite = function()
        sprite = Sprite:new("media/sprites/dumblock.png", {32, 32})

        return sprite
    end
})
Box = _.Box
