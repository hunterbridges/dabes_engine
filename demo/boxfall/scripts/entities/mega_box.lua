require 'entities.box'

MegaBox = Box:extend({
	init = function(self)
        local body = Body:new("chipmunk", 4.0, 4.0, 100, true)
        body.friction = 1
        body.elasticity = 0
        self.body = body

        self.sprite = self.build_sprite()
    end
})