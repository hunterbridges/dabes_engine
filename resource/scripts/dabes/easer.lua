require 'dabes.bound_object'

Easer = BoundObject:extend({
    lib = dab_easer,

-- Hook Overloads

    realize = function(class, length_ms)
        return class.lib.new(length_ms)
    end,

-- Bound Functions

    _getters = {
        value = BoundObject.fwd_func("get_value"),
        time_scale = BoundObject.fwd_func("get_time_scale")
    },

    _setters = {
        value = BoundObject.fwd_func("set_value"),
        time_scale = BoundObject.fwd_func("set_time_scale")
    },

-- Hooks

    update = function(easer) end,

    finish = function(easer) end

})
