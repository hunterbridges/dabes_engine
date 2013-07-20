--- An object for modifying values over time.
--
-- An `Easer` starts advancing through time on the frame following its creation.
--
-- @{easer|Easer} extends @{bound_object|BoundObject}
--
-- @module easer
-- @type Easer

require 'dabes.bound_object'

Easer = BoundObject:extend({
    lib = dab_easer,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- A value from `0` to `1` indicating the progress of the `Easer`,
        -- with the easing curve applied.
        value = BoundObject.fwd_func("get_value"),

        --- Coefficient of how the advance of time affects the progress of
        -- the `Easer`.
        --
        -- Default is `1`
        time_scale = BoundObject.fwd_func("get_time_scale")
    },

    _setters = {
        value = BoundObject.fwd_func("set_value"),
        time_scale = BoundObject.fwd_func("set_time_scale")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Easer`
    --
    -- @name Easer:new
    -- @tparam number length_ms Duration of the `Easer` in milliseconds.
    realize = function(class, length_ms)
        return class.lib.new(length_ms)
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called every frame after `value` has updated.
    -- @tparam Easer self The `Easer` instance
    update = function(easer) end,

    --- Called when the `Easer` has completed.
    -- @tparam Easer self The `Easer` instance
    finish = function(easer) end

})
