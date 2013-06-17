require 'boxfall.entities.box'
require 'dabes.entity'
require 'dabes.body'
require 'dabes.sensor'
require 'dabes.sfx'
require 'dabes.sprite'
inspect = require 'lib.inspect'

Megaman = Entity:extend({
    body_type = "chipmunk",

    init = function(self)
        self.holding_up = false

        local w = 1.0
        local h = 1.0
        local body = Body:new(self.body_type, w, h, 100, false)
        local hbw = 21.0 / 32.0
        local hbh = 24.0 / 32.0
        body:set_hit_box(hbw, hbh, {0.0, h * 4.0 / 32.0})
        body.friction = 0.7
        body.elasticity = 0
        self.body = body

        self.ground_sensor = Sensor:new(hbw * w - 0.2, 0.1,
                                        {0, hbh / 2 * h - 0.04})
        body:add_sensor(self.ground_sensor)

        self.sprite = self.build_sprite()
        self.alpha = 0
    end,

    build_sprite = function()
        local sprite = Sprite:new("media/sprites/megaman.png", {32, 32}, 1)

        local standing = SpriteAnimation:new(0)
        standing.fps = 0
        sprite:add_animation(standing, "standing")

        local running = SpriteAnimation:new(4, 5, 6, 5)
        running.fps = 7
        sprite:add_animation(running, "running")

        local jumping = SpriteAnimation:new(12)
        jumping.fps = 0
        sprite:add_animation(jumping, "jumping")

        local turning = SpriteAnimation:new(8, 9, 10, 11)
        turning.fps = 7
        turning.repeats = false
        sprite:add_animation(turning, "turning")

        sprite:use_animation("standing")

        return sprite
    end,

    main = function(self)
        if self.controller.a_button then
            local on_s = self.ground_sensor.on_sensors
            for i = 1, #on_s do
                local other_s = on_s[i]
                local other_e = other_s.body.entity
                if other_e ~= nil and other_e.open ~= nil then
                    local anim = self.sprite:get_animation("turning")
                    self.opening = true
                    self.body.is_rogue = true

                    anim.complete = function(self)
                        other_e:open()
                        anim.complete = function() end
                    end
                end
            end
        end

        local old_hup = self.holding_up
        if self.controller.up then
            self.holding_up = true
        else
            self.holding_up = false
        end
        if old_hup ~= self.holding_up and self.holding_up == true then
            local projectile = Box:new()
            local ppos = self.body.pos
            ppos[2] = ppos[2] - 1.0
            projectile.body.pos = ppos
            projectile.body.velo = {0, -15}
            projectile.body.elasticity = 0.1
            projectile.body.mass = 1000
            self.scene:add_entity(projectile)
        end

        self:control()
        self:derive_animation()
    end,

    motion = {
        run_accel = 10,
        air_accel = 2 * 10,
        turn_accel = 3 * 10,
        max_velo = 10,
        jump_velo_hi = -8,
        jump_velo_low = -4
    },

    control = function(self)
        if self.opening then return end

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
        if self.opening then
            self.sprite:use_animation('turning')
            return
        end

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
