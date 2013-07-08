require 'entities.box'

SquiggyBox = Box:extend({
    main = function(self)
        local dt = math.floor((dab_engine.ticks() - self.born_at) / 1000)
        if (dt < 4) then return end

        local force_x = 0
        if dt % 2 == 0 then
            force_x = 5
        else
            force_x = -5
        end

        local velo_x = self.body.velo[1]

        if velo_x * force_x > 0 then
          if math.abs(velo_x) >= 15 then
              force_x = 0
          end
        else
          force_x = force_x * 3
        end

        self.body:apply_force({force_x * self.body.mass, 0},
                              {0, 0})
    end
})
