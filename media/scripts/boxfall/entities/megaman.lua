require 'dabes.entity'
require 'dabes.body'
require 'dabes.sprite'

Megaman = Entity:extend({
    body_type = "chipmunk",

    init = function(self)
        local body = Body:new(self.body_type, 2.0, 2.0, 100, false)
        body.friction = 0.7
        self.body = body

        self.sprite = self.build_sprite()
        self.alpha = 0
    end,

    build_sprite = function()
        local sprite = Sprite:new("media/sprites/megaman_run.png", {32, 32})

        local standing = SpriteAnimation:new(0)
        standing.fps = 0
        sprite:add_animation(standing, "standing")

        local running = SpriteAnimation:new(1, 2, 3, 2)
        running.fps = 7
        sprite:add_animation(running, "running")

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
        local on_ground = true -- TODO
        local velo = self.body.velo
        local input_accel = {0, 0}
        if self.controller.right then
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

        if self.controller.left then
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

        if self.controller.a_button then
            if on_ground then
                -- TODO: Jump sound
                velo[2] = self.motion.jump_velo_hi
            end
        else
            if not on_ground then
                if velo[2] < self.motion.jump_velo_low then
                    velo[2] = self.motion.jump_velo_lo
                end
            end
        end

        if (math.abs(velo[1]) >= self.motion.max_velo and
                velo[1] * input_accel[1] > 0) then
            input_accel[1] = 0
        end

        self.body.velo = velo

        local mass = self.body.mass
        local input_force = {input_accel[1] * mass, input_accel[2] * mass}
        self.body.force = input_force
    end,

    derive_animation = function(self)
        local velo = self.body.velo
        local standing_thresh = 0.25
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
