-- Controller
--
-- This BoundObject represents a Controller (controller.h)
require 'dabes.bound_object'

Controller = BoundObject.extend({
    lib = dabes_controller,

-- Forwarded Methods
    _getters = {
        up = BoundObject.fwd_func('is_up'),
        down = BoundObject.fwd_func('is_down'),
        left = BoundObject.fwd_func('is_left'),
        right = BoundObject.fwd_func('is_right'),
        a_button = BoundObject.fwd_func('is_jumping')
    },

    _setters = {
        up = BoundObject.readonly,
        down = BoundObject.readonly,
        left = BoundObject.readonly,
        right = BoundObject.readonly,
        a_button = BoundObject.readonly
    }
})
