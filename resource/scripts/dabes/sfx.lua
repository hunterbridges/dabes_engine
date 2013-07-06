-- Sfx
--
-- This BoundObject represents a Sfx (sfx.h)

require 'dabes.bound_object'

Sfx = BoundObject:extend({
    lib = dab_sfx,

-- Hook Overloads

    realize = function(class, filename)
        return class.lib.new(filename)
    end,

-- Function Bindings

    -- play(self)
    --
    -- Plays the music
    play = BoundObject.fwd_func('play'),

    _getters = {
        volume = BoundObject.fwd_func("get_volume")
    },

    _setters = {
        volume = BoundObject.fwd_func("set_volume")
    }
})
