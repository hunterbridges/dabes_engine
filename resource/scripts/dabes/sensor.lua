--- Adds qualitative collision detection to a @{body|Body}.
--
-- A `Sensor` is simply a rectangular area that is attached to and moves with a
-- @{body|Body}, but does not affect the physics of it. `Sensors` can detect
-- other `Sensors` they are overlapping. They can also detect whether they are
-- overlapping the map.
--
-- @{sensor|Sensor} extends @{bound_object|BoundObject}
--
-- @module sensor
-- @type Sensor

require 'dabes.bound_object'

Sensor = BoundObject:extend({
    lib = dab_sensor,

-- Default Configuration

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Sensor`
    --
    -- @function Sensor:new
    -- @tparam number width The width of the sensor in meters
    -- @tparam number height The height of the sensor in meters
    -- @tparam table offset An `{x, y}` vector of the offset from the center
    --   of the @{body|Body} to which `Sensor` is attached. In meters.
    -- @treturn Sensor
    realize = function(class, w, h, offset)
        return class.lib.new(w, h, offset)
    end,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- **(bool)** *(read only)* Whether the `Sensor` is currently
        -- overlapping the map.
        on_static = BoundObject.fwd_func('get_on_static'),

        --- *(read only)* A list of other `Sensors` this `Sensor` is currently
        -- overlapping with.
        on_sensors = BoundObject.fwd_func('get_on_sensors'),

        --- *(read only)* The `Body` this `Sensor` is attached to.
        body = BoundObject.fwd_func('get_body')
    },

    _setters = {
        on_static = BoundObject.readonly,
        on_sensors = BoundObject.readonly,
        body = BoundObject.readonly
    }
})
