--- Enables a @{canvas|Canvas} to detect drawn shapes by letting the user
-- play a dynamic game of connect-the-dots. I'll think of a better explanation
-- later.
--
-- @{shape_matcher|ShapeMatcher} extends @{bound_object|BoundObject}
--
-- @module shape_matcher
-- @type ShapeMatcher

require 'dabes.bound_object'
require 'dabes.shape'

ShapeMatcher = BoundObject:extend({
    lib = dab_shape_matcher,

--- Constants.
-- Accessed at the class level e.g. `Class.CONSTANT`
-- @section constants

    --- A ShapeMatcher state in which it is not running and has no data.
    -- @class field
    STATE_NEW = 0,

    --- A ShapeMatcher state in which it is running and gathering data.
    -- @class field
    STATE_RUNNING = 1,

    --- A ShapeMatcher state in which it is no longer running and has data.
    -- @class field
    STATE_ENDED = 2,

    --- Denotes a counterclockwise winding.
    -- @class field
    WINDING_COUNTERCLOCKWISE = 1,

    --- Denotes an ambiguous winding.
    -- @class field
    WINDING_AMBIGUOUS = 0,

    --- Denotes a clockwise winding.
    -- @class field
    WINDING_CLOCKWISE = -1,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- _(readonly)_ The current ShapeMatcher state
        -- @see STATE_NEW
        -- @see STATE_RUNNING
        -- @see STATE_ENDED"
        -- @class field
        state = BoundObject.fwd_func("get_state"),

        --- The coefficient by which "dot" radii are calculated.
        -- The actual radius is figured by multiplying
        -- _vertex\_catch\_tolerance_ and _initial\_segment\_length_.
        --
        -- Default `0.2`
        -- @class field
        vertex_catch_tolerance =
            BoundObject.fwd_func("get_vertex_catch_tolerance"),

        --- The coefficient by which a sloppy line is considered too
        -- long for the approach to the next "dot".
        --
        -- Default `1.1`
        -- @class field
        slop_tolerance =
            BoundObject.fwd_func("get_slop_tolerance"),

        --- _(readonly)_ The absolute trajectory angle of the first segment
        -- in degrees. `0` degrees points right.
        --
        -- If no segment has been drawn, this is `0`.
        --
        -- @class field
        initial_segment_angle =
            BoundObject.fwd_func("get_initial_segment_angle"),

        --- _(readonly)_ The length of the initial segment, in pixels.
        --
        -- If no segment has been drawn, this is `0`.
        --
        -- @class field
        initial_segment_length =
            BoundObject.fwd_func("get_initial_segment_length"),

        --- _(readonly)_ The detected winding of the shape the user is
        -- drawing.
        -- @see WINDING_COUNTERCLOCKWISE
        -- @see WINDING_AMBIGUOUS
        -- @see WINDING_CLOCKWISE
        -- @class field
        intended_convex_winding =
            BoundObject.fwd_func("get_intended_convex_winding"),

        --- **(bool)** Whether to render a debug overlay that reveals all of
        -- the shapes. Helpful when designing new shapes.
        -- @class field
        debug_shapes = BoundObject.fwd_func("get_debug_shapes"),

        --- An `{r, g, b, a}` vector representing the `debug_shapes` render
        -- color.
        -- @class field
        debug_shape_color = BoundObject.fwd_func("get_debug_shape_color"),

        --- The width of the `debug_shapes` stroke.
        -- @class field
        debug_shape_width = BoundObject.fwd_func("get_debug_shape_width"),

        --- An `{r, g, b, a}` vector representing the dot color.
        -- @class field
        dot_color = BoundObject.fwd_func("get_dot_color"),

        --- The width of the dot stroke.
        dot_width = BoundObject.fwd_func("get_dot_width")
    },

    _setters = {
        state = BoundObject.readonly,

        --- _(readonly)_ The @{shape|Shapes} being matched
        -- @class field
        shapes = BoundObject.readonly,

        vertex_catch_tolerance =
            BoundObject.fwd_func("set_vertex_catch_tolerance"),
        slop_tolerance =
            BoundObject.fwd_func("set_slop_tolerance"),
        initial_segment_angle = BoundObject.readonly,
        initial_segment_length = BoundObject.readonly,
        intended_convex_winding = BoundObject.readonly,
        debug_shapes = BoundObject.fwd_func("set_debug_shapes"),
        debug_shape_color = BoundObject.fwd_func("set_debug_shape_color"),
        debug_shape_width = BoundObject.fwd_func("set_debug_shape_width"),
        dot_color = BoundObject.fwd_func("set_dot_color"),
        dot_width = BoundObject.fwd_func("set_dot_width")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `ShapeMatcher` intended to match the given shapes.
    --
    -- @function ShapeMatcher:new
    -- @tparam number ... A variable list of @{shape|Shapes} to match.
    -- @treturn ShapeMatcher
    -- @usage local matcher = ShapeMatcher:new(square, zigzag, triangle)
    realize = function(class, ...)
        return class.lib.new(map_real(...))
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    init = function(self, ...)
        rawset(self, "shapes", table.pack(...))
    end,

    --- Called when the ShapeMatcher changes its `state`
    --
    -- @function state_changed
    -- @tparam ShapeMatcher self The `ShapeMatcher` instance
    -- @param state The new state
    state_changed = function(self, state)
    end,

    --- Called when the ShapeMatcher matches a shape successfully
    --
    -- @function matched
    -- @tparam ShapeMatcher self The `ShapeMatcher` instance
    -- @tparam Shape shape The shape that was matched
    -- @param accuracy A number (percentage) between `0` and `1` reflecting how
    -- accurately the user matched the shape.
    matched = function(self, shape, accuracy)
    end,

    --- Called when the ShapeMatcher fails to match a shape.
    --
    -- @function failed
    -- @tparam ShapeMatcher self The `ShapeMatcher` instance
    failed = function(self)
    end
})

