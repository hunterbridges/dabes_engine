--- Manages a sequence of update callbacks over time.
--
-- @{step_sequencer|StepSequencer} extends @{object|Object}
--
-- @module step_sequencer
-- @type StepSequencer

require 'dabes.object'

StepSequencer = Object:extend({

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `StepSequencer`
    -- @function StepSequencer:new
    -- @tparam table ... the steps to sequence, of types `wait`, `every`, or
    -- `until`
    -- @treturn StepSequencer
    new = function(klass, ...)
        local self = Object.new(klass)

        self.count = 0
        self.all_steps = {}
        self.pending_steps = {}

        for i = 1, select('#', ...) do
            local v = select(i, ...)

            if type(v) ~= 'table' and type(v) ~= 'function' then
                error("Argument "..i.." is not a table", 2)
            end

            local prepped = {}

            if type(v) == 'function' then
                prepped.steps = 0
                prepped.kind = 'wait'
                prepped.update = v
            elseif type(v) == 'table' then
                local key = v[1]
                if key == "wait" then
                    prepped.steps = v[2]
                    prepped.kind = key
                    prepped.update = v[3]

                elseif key == "every" then
                    prepped.steps = v[2]
                    prepped.kind = key
                    prepped.update = v[3]

                elseif key == "until" then
                    prepped.kind = key
                    prepped.update = v[2]

                else
                    error("Invalid key `"..key.."` in step "..i)
                end
            end

            table.insert(self.all_steps, prepped)
            table.insert(self.pending_steps, prepped)
        end

        return self
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Run the StepSequencer.
    -- @function step_sequener:update
    -- @tparam number steps How many steps to advance the sequencer.
    -- @treturn boolean `true` if the StepSequencer has pending cycles, `false`
    -- if it has completed.
    update = function(self, steps)
        local running = true
        local work = true

        while work do
            -- Have a current step?
            if self.current_step ~= nil then
                if self.current_step.kind == 'every' then
                    local tween = (self.count - self.current_step.started_at) /
                                  self.current_step.steps
                    self.current_step.update(self, self.current_env, tween)
                elseif self.current_step.kind == 'until' then
                    local ret = self.current_step.update(self, self.current_env)

                    if ret then
                        self.current_step = nil
                    else
                        work = false
                    end
                end

                if self.current_step and
                        self.current_step.end_at and
                        self.current_step.end_at <= self.count then

                    if self.current_step.kind == 'wait' and
                            self.current_step.update then
                        self.current_step.update(self, self.current_env)
                    end

                    self.current_step = nil
                else
                    -- We know we need to stay on this step.
                    work = false
                end
            end

            if self.current_step == nil then
                -- Have a pending step?
                if #self.pending_steps > 0 then
                    self.current_step = table.remove(self.pending_steps, 1)
                    self.current_step.started_at = self.count

                    if self.current_step.kind == 'every' or
                            self.current_step.kind == 'wait' then
                        self.current_step.end_at =
                            self.count + self.current_step.steps
                    end

                    self.current_env = {}
                else
                    running = false
                    work = false
                end
            end
        end

        self.count = self.count + steps

        return running
    end
})
