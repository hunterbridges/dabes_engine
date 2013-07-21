--- A one-shot OGG Vorbis audio sample.
--
-- @{sfx|Sfx} extends @{bound_object|BoundObject}
--
-- @module sfx
-- @type Sfx

require 'dabes.bound_object'

Sfx = BoundObject:extend({
    lib = dab_sfx,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- Minimum `0`, maximum `1`
        volume = BoundObject.fwd_func("get_volume")
    },

    _setters = {
        volume = BoundObject.fwd_func("set_volume")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Sfx` by loading an OGG file
    --
    -- @function Sfx:new
    -- @tparam string filename The filename of the OGG file
    -- @treturn Sfx
    realize = function(class, filename)
        local realized = class.lib.new(filename)
        if realized == nil then
            error("Sfx: OGG file invalid or not found.", 3)
        end

        return realized
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Play the `Sfx`
    --
    -- @function sfx:play
    -- @treturn nil
    play = BoundObject.fwd_func('play'),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when the `Music` ends.
    -- @tparam Music self The instance
    ended = function(self) end,

})
