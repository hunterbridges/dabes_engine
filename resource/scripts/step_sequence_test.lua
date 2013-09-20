require 'dabes.step_sequencer'

local sequence = StepSequencer:new(
    function(seq, env)
        print "Starting with this"
    end,

    {"every", 10, function(seq, env, tween)
        print("Tween "..tween)
    end},

    function(seq, env)
        print "Shorthand for wait 0"
    end,

    {"wait", 10},

    function(seq, env)
        print "Ok waited"
    end,

    {"until", function(seq, env)
        if env.count == nil then
            env.count = 0
        end

        env.count = env.count + 1

        return env.count >= 40
    end}
)

while sequence:update(1) do end

print "end of the road, sonnyboy"
