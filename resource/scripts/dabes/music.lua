-- Music
--
-- This BoundObject represents a Music (music.h)

require 'dabes.bound_object'

Music = BoundObject:extend({
    lib = dab_music,

-- Hook Overloads

    realize = function(class, ...)
        local realized = class.lib.new(...)
        if realized == nil then
            error("Music: OGG file invalid or not found.", 3)
        end

        return realized
    end,

-- Function Bindings

    -- play(self)
    --
    -- Plays the music
    play = BoundObject.fwd_func("play"),

    -- pause(self)
    --
    -- Pauses the music
    pause = BoundObject.fwd_func("pause"),

    -- stop(self)
    --
    -- Stops the music
    stop = BoundObject.fwd_func("stop"),

    _getters = {
        volume = BoundObject.fwd_func("get_volume")
    },

    _setters = {
        volume = BoundObject.fwd_func("set_volume")
    }
})
