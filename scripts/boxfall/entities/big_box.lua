require 'boxfall.entities.box'

BigBox = Box:extend({
	init = function(self)
        local body = Body:new("chipmunk", 2.0, 2.0, 10, true)
        body.friction = 0.5
        body.elasticity = 0
        self.body = body

        self.sprite = self.build_sprite()
    end
})