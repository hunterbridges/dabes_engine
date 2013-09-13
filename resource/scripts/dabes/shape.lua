--- Defines a shape for use in a `ShapeMatcher`.
--
-- A shape must be built from line segments and does not support curves.
--
-- @{shape|Shape} extends @{bound_object|BoundObject}
--
-- @module shape
-- @type Shape

require 'dabes.bound_object'

Shape = BoundObject:extend({
    lib = dab_shape,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- _(readonly)_ The name of the shape.
        -- @class field
        name = BoundObject.fwd_func("get_name"),
    },

    _setters = {
        name = BoundObject.readonly,

        --- _(readonly)_ The segments that make up the shape.
        -- This is a sequence of tables.
        --
        -- A segment is a table like `{length = 1, angle = 90}`
        -- @class field
        segments = BoundObject.readonly,
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Shape` from a name and segment definitions.
    --
    -- The shape definition always omits the initial segment.
    --
    -- @function SpriteAnimation:new
    -- @tparam string name A name for the shape.
    -- @tparam table ... A sequence of tables with keys `length` and `angle`.
    -- `length` is the length of the segment relative to the first segment.
    -- `angle` is the angle in degrees at which to extend the segment from the
    -- one before it based on the convex winding direction.
    -- @treturn Shape
    -- @usage
    --    local square = Shape:new("Square",
    --                             { length = 1, angle = 90},
    --                             { length = 1, angle = 90},
    --                             { length = 1, angle = 90})
    realize = function(class, name, ...)
        return class.lib.new(name, ...)
    end,

    init = function(self, name, ...)
        self.segments = table.pack(...)
    end
})
