--- Records and replays the motion of an @{entity|Entity}
--
-- @{recorder|Recorder} extends @{bound_object|BoundObject}
--
-- @module canvas
-- @type Canvas

Recorder = BoundObject:extend({
    lib = dab_recorder,

--- Constants.
-- Accessed at the class level e.g. `Class.CONSTANT`
-- @section constants

    --- A Recorder state in which it is idle/paused.
    -- @class field
    STATE_IDLE = 0,

    --- A Recorder state in which it is recording.
    --
    -- Note that if a recorder is recording, it is actively gathering
    -- data. Leaving a recorder recording accidentally will leak memory.
    --
    -- @class field
    STATE_RECORDING = 1,

    --- A Recorder state in which it playing.
    -- @class field
    STATE_PLAYING = 2,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- _(readonly)_ The current Recorder state
        -- @see STATE_IDLE
        -- @see STATE_RECORDING
        -- @see STATE_PLAYING
        -- @class field
        state = BoundObject.fwd_func("get_state"),

        --- The @{entity|Entity} being captured or controlled by
        -- the recorder.
        entity = BoundObject.fwd_func("get_entity"),
    },

    _setters = {
        entity = BoundObject.fwd_func("set_entity"),
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Recorder` instance.
    --
    -- @function Recorder:new
    -- @tparam number preroll_ms The amount of preroll for the recorder.
    -- (Currently unused)
    -- @treturn Recorder
    realize = function(class, preroll_ms)
        return class.lib.new(preroll_ms)
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Play the @{recorder|Recorder}.
    --
    -- @function recorder:play
    -- @treturn nil
    play = BoundObject.fwd_func("play"),

    --- Pause the @{recorder|Recorder}.
    --
    -- @function recorder:pause
    -- @treturn nil
    pause = BoundObject.fwd_func("pause"),

    --- Rewind the @{recorder|Recorder}.
    --
    -- @function recorder:rewind
    -- @treturn nil
    rewind = BoundObject.fwd_func("rewind"),

    --- Start recording. **This actively gathers data and leaks memory if
    -- left unpaused.**
    --
    -- @function recorder:record
    -- @treturn nil
    record = BoundObject.fwd_func("record"),

    --- Clear the recorded contents. This frees any memory allocated by
    -- `record`.
    --
    -- @function recorder:clear
    -- @treturn nil
    clear = BoundObject.fwd_func("clear"),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when the Recorder has reached the end of playback.
    --
    -- @function finished_playing
    -- @tparam Recorder self The `Recorder` instance
    finished_playing = function(self) end
})
