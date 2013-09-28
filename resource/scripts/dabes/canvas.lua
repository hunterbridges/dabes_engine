--- A layer the user can draw on
--
-- @{canvas|Canvas} extends @{bound_object|BoundObject}
--
-- @module canvas
-- @type Canvas

require 'dabes.bound_object'

Canvas = BoundObject:extend({
    lib = dab_canvas,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- Alpha value of the canvas from `0` to `1`
        alpha = BoundObject.fwd_func("get_alpha"),

        --- The angle difference required for the path simplifier to recognize
        -- a new vertex. In degrees
        angle_threshold = BoundObject.fwd_func("get_angle_threshold"),

        --- An `{r, g, b, a}` vector representing the color for the line
        -- between the first and last point of the draw action.
        angle_color = BoundObject.fwd_func("get_angle_color"),

        --- An `{r, g, b, a}` vector representing the Canvas background color.
        bg_color = BoundObject.fwd_func("get_bg_color"),

        --- The minimum distance required for the path simplifier to recognize
        -- a new vertex. In pixels
        distance_threshold = BoundObject.fwd_func("get_distance_threshold"),

        --- An `{r, g, b, a}` vector representing the Canvas draw color.
        draw_color = BoundObject.fwd_func("get_draw_color"),

        --- Draw render width in pixels
        draw_width = BoundObject.fwd_func("get_draw_width"),

        --- **(bool)** Whether the canvas is currently accepting input.
        enabled = BoundObject.fwd_func("get_enabled"),

        --- An `{r, g, b, a}` vector representing the simplified path.
        simplified_path_color =
            BoundObject.fwd_func("get_simplified_path_color"),

        --- A @{shape_matcher|ShapeMatcher} hosted by the Canvas
        shape_matcher =
            BoundObject.fwd_func("get_shape_matcher"),
    },

    _setters = {
        alpha = BoundObject.fwd_func("set_alpha"),
        angle_threshold = BoundObject.fwd_func("set_angle_threshold"),
        angle_color = BoundObject.fwd_func("set_angle_color"),
        bg_color = BoundObject.fwd_func("set_bg_color"),
        distance_threshold = BoundObject.fwd_func("set_distance_threshold"),
        draw_color = BoundObject.fwd_func("set_draw_color"),
        draw_width = BoundObject.fwd_func("set_draw_width"),
        enabled = BoundObject.fwd_func("set_enabled"),
        simplified_path_color =
            BoundObject.fwd_func("set_simplified_path_color"),
        shape_matcher =
            BoundObject.fwd_func_real("set_shape_matcher"),
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Canvas`
    --
    -- @function Canvas:new
    -- @treturn Canvas
    realize = function(class)
        local realized = class.lib.new()

        return realized
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Empty the canvas.
    --
    -- @function canvas:empty
    -- @treturn nil
    empty = BoundObject.fwd_func("empty"),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when the Canvas finishes drawing
    --
    -- @function finish
    -- @tparam Canvas self The `Canvas` instance
    -- @param angle The angle between the touch and release points in degrees
    -- @param distance The distance between the touch and release points in
    -- pixels
    finish = function(self, angle, distance)
        print("Finished Canvas - Angle: "..angle..", Distance: "..distance)
    end,
})
