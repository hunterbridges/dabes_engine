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
    end,

    main = function(self)
        self:control()
        self:derive_animation()
    end,

    motion = {
        run_accel = 15,
        air_accel = 2 * 15,
        turn_accel = 3 * 15,
        max_velo = 15,
        jump_velo_hi = -8,
        jump_velo_low = -4
    },

    control = function(self)
        if self.controller == nil then return end
        on_ground = true -- TODO
        velo = self.body.velo
        input_accel = {0, 0}
        if self.controller.is_right then
            if on_ground then
                if velo[1] < 0 then
                    input_accel[1] = self.motion.turn_accel
                else
                    input_accel[1] = self.motion.run_accel
                end
            else
                input_accel[1] = self.motion.air_accel
            end
        end

        if self.controller.is_left then
            if on_ground then
                if velo[1] > 0 then
                    input_accel[1] = -self.motion.turn_accel
                else
                    input_accel[1] = -self.motion.run_accel
                end
            else
                input_accel[1] = -self.motion.air_accel
            end
        end

        if self.controller.is_jumping then
            if on_ground then
                -- TODO: Jump sound
                velo[2] = self.motion.jump_velo_hi
            end
        else
            if velo[2] < self.motion.jump_velo_low then
                velo[2] = self.motion.jump_velo_lo
            end
        end

        if (math.abs(velo[1]) >= self.motion.max_velo and
                velo[1] * input_accel[1] > 0) then
            input_accel[1] = 0
        end

        self.body.velo = velo

        mass = self.body.mass
        input_force = {input_accel[1] * mass, input_accel[2] * mass}
        self.body:apply_force(input_force, {0, 0.1})
    end,

    derive_animation = function(self)
        velo = self.body.velo
        standing_thresh = 0.25
        if velo[1] < -standing_thresh then
            self.sprite.direction = 180
        elseif velo[1] > standing_thresh then
            self.sprite.direction = 0
        end

        if velo[1] > -standing_thresh and velo[1] < standing_thresh then
            self.sprite:use_animation('standing')
        else
            self.sprite:use_animation('running')
        end
    end
})
