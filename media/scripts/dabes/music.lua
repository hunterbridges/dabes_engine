-- Music
--
-- This BoundObject represents a Music (music.h)

require 'dabes.bound_object'

Music = BoundObject:extend({
    lib = dab_music,

-- Hook Overloads

    realize = function(class, ...)
        return class.lib.new(...)
    end,

-- Function Bindings

    -- play(self)
    --
    -- Plays the music
    play = BoundObject.fwd_func("play"),

    -- pause(self)
    --
    -- Pauses the music
    pause = BoundObject.fwd_func("pause")
})
