--- A controllable OGG Vorbis audio stream.
--
-- @{music|Music} extends @{bound_object|BoundObject}
--
-- @module music
-- @type Music

require 'dabes.bound_object'

Music = BoundObject:extend({
    lib = dab_music,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- Minimum `0`, maximum `1`
        volume = BoundObject.fwd_func("get_volume"),

        --- **(bool)** Whether the music should loop.
        loop = BoundObject.fwd_func("get_loop")
    },

    _setters = {
        volume = BoundObject.fwd_func("set_volume"),
        loop = BoundObject.fwd_func("set_loop")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Music` instance
    --
    -- @name Music:new
    -- @tparam string ... A variable list of OGG files to load as a sequence.
    -- The music will play starting from the first file. The last file will be
    -- looped if `loop` is `true`.
    -- @treturn Music
    -- @usage scene.music = Music:new("media/music/intro.ogg",
    --                        "media/music/loop.ogg")
    realize = function(class, ...)
        local realized = class.lib.new(...)
        if realized == nil then
            error("Music: OGG file invalid or not found.", 3)
        end

        return realized
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Play the `Music`
    -- @function music:play
    -- @treturn nil
    play = BoundObject.fwd_func("play"),

    --- Pause the `Music`
    -- @function music:pause
    -- @treturn nil
    pause = BoundObject.fwd_func("pause"),

    --- Stop the `Music` and rewind
    -- @function music:stop
    -- @treturn nil
    stop = BoundObject.fwd_func("stop"),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when the `Music` ends.
    --
    -- This is only called if `loop` is `false`
    --
    -- @tparam Music self The instance
    ended = function(self) end,

})
