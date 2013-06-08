require 'dabes.entity'
require 'dabes.body'
require 'dabes.sensor'
require 'dabes.sfx'
require 'dabes.sprite'

Megaman = Entity:extend({
    body_type = "chipmunk",

    init = function(self)
        local w = 2.0
        local h = 2.0
        local body = Body:new(self.body_type, w, h, 100, false)
        local hbw = 21.0 / 32.0
        local hbh = 24.0 / 32.0
        body:set_hit_box(hbw, hbh, {0.0, h * 4.0 / 32.0})
        body.friction = 0.7
        self.body = body

        self.ground_sensor = Sensor:new(hbw * w / 2, 0.2, {0, hbh / 2 * h - 0.08})
        body:add_sensor(self.ground_sensor)

        self.sprite = self.build_sprite()
        self.alpha = 0
    end,

    build_sprite = function()
        local sprite = Sprite:new("media/sprites/megaman.png", {32, 32})

        local standing = SpriteAnimation:new(0)
        standing.fps = 0
        sprite:add_animation(standing, "standing")

        local running = SpriteAnimation:new(1, 2, 3, 2)
        running.fps = 7
        sprite:add_animation(running, "running")

        local jumping = SpriteAnimation:new(4)
        jumping.fps = 0
        sprite:add_animation(jumping, "jumping")

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
        jump_velo_hi = -10,
        jump_velo_low = -5
    },

    control = function(self)
        local on_ground = self.ground_sensor.on_static
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
                local jump_sound = Sfx:new("media/sfx/jump.ogg")
                jump_sound.volume = 2.0
                jump_sound:play()

                velo[2] = self.motion.jump_velo_hi
            end
        else
            if not on_ground then
                if velo[2] < self.motion.jump_velo_low then
                    velo[2] = self.motion.jump_velo_low
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
        local on_ground = self.ground_sensor.on_static

        local velo = self.body.velo
        local standing_thresh = 0.25
        if velo[1] < -standing_thresh then
            self.sprite.direction = 180
        elseif velo[1] > standing_thresh then
            self.sprite.direction = 0
        end

        if not on_ground then
            self.sprite:use_animation('jumping')
            return
        end

        if velo[1] > -standing_thresh and velo[1] < standing_thresh then
            self.sprite:use_animation('standing')
        else
            self.sprite:use_animation('running')
        end
    end
})
