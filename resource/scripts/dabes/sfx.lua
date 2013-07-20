--- A one-shot OGG Vorbis audio sample.
--
-- @{sfx|Sfx} extends @{bound_object|BoundObject}
--
-- @module sfx
-- @type Sfx

require 'dabes.bound_object'

Sfx = BoundObject:extend({
    lib = dab_sfx,

-- Hook Overloads

    realize = function(class, filename)
        local realized = class.lib.new(filename)
        if realized == nil then
            error("Sfx: OGG file invalid or not found.", 3)
        end

        return realized
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
    },

-- Hooks

    -- ended
    --
    -- This will be called when the Sfx ends.

    ended = function(self) end,

})
