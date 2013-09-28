--- A layer that sprites or text can manually be drawn in.
--
-- @{overlay|Overlay} extends @{bound_object|BoundObject}
--
-- @module overlay
-- @type Overlay

require 'dabes.bound_object'

Overlay = BoundObject:extend({
    lib = dab_overlay,

--- Configuration.
-- Required by subclass declarations. Used when instantiating
-- concrete subclasses.
-- @section configuration

    --- The file name of the TTF or OTF font to use when drawing strings.
    --
    -- Defaults to `nil`
    font_name = nil,

    --- The font size in pixels to use when drawing strings.
    --
    -- Defaults to `0`
    font_size = 0,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    _getters = {
        --- *(read only)* The @{scene|Scene} that contains `Overlay`
        scene = BoundObject.fwd_func("get_scene"),

        --- Just like in CSS, the highest `z_index` gets drawn in front.
        --
        -- Overlays are always drawn in front of @{entity|Entities}.
        z_index = BoundObject.fwd_func("get_z_index"),

        --- The @{entity|Entity} tracked by `Overlay`.
        --
        -- This will make point ```{0, 0}``` `track_entity_edge`'s point on the
        -- `track_entity`'s bounding box.
        track_entity = BoundObject.fwd_func("get_track_entity"),

        --- The edge of the `track_entity` used to determine ```{0, 0}```
        --
        --
        -- The edges are relative to the bounding box as follows:
        --     0---1---2
        --     |   |   |
        --     3---4---5
        --     |   |   |
        --     6---7---8
        --
        -- (```0``` is top left, ```4``` is center, ```8``` is bottom right)
        track_entity_edge = BoundObject.fwd_func("get_track_entity_edge")
    },

    _setters = {
        scene = BoundObject.readonly,
        z_index = BoundObject.fwd_func("set_z_index"),
        track_entity = BoundObject.fwd_func_real("set_track_entity"),
        track_entity_edge = BoundObject.fwd_func("set_track_entity_edge")
    },

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Overlay`
    --
    -- @function Overlay:new
    -- @treturn Overlay
    realize = function(class)
        local realized = class.lib.new(class.font_name, class.font_size)
        if realized == nil then
            error("Overlay: Unable to open font `" .. font_name .. "`", 3)
        end

        return realized
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Add a @{sprite|Sprite} to the `Overlay`.
    --
    -- The main reason for this is so `sprite` can be updated.
    -- If you do not do this, @{sprite_animation|SpriteAnimations} will not
    -- advance.
    --
    -- @function overlay:add_sprite
    -- @tparam Sprite sprite The sprite to add
    -- @treturn nil
    add_sprite = BoundObject.fwd_adder("add_sprite"),

    --- Draw a string to `Overlay`.
    --
    -- This function is called with an "`opts`" parameter table. The parameters
    -- do not have to be in order.
    --
    -- **Valid Options:**
    --
    -- * `string` = The string to draw. *(required)*
    --
    -- * `color` = `{r, g, b, a}` vector representing the text color.
    -- Default `{1, 1, 1, 1}` (white).
    --
    -- * `origin` = `{x, y}` baseline origin point relative to the
    -- screen center or entity edge in pixels. Default `{0, 0}`
    --
    -- * `align` = The text alignment. Can be `"left"`, `"right"` or
    -- `"center"`. Default `"left"`
    --
    -- * `shadow_color` = `{r, g, b, a}` vector representing the shadow
    -- color. Default `nil`.
    --
    -- * `shadow_offset` = `{x, y}` vector representing the shadow
    -- offset in pixels. Default `nil`
    --
    -- @function overlay:draw_string
    -- @tparam table opts The named parameters.
    -- @treturn nil
    --
    -- @usage
    -- -- Red text with a white shadow, drawn in the center of the screen.
    -- overlay:draw_string({
    --     string = "Hello World!",
    --     align = "center",
    --     color = {1, 0, 0, 1},
    --     origin = {0, 0},
    --     shadow_color = {1, 1, 1, 1},
    --     shadow_offset = {0, 0}
    -- })
    draw_string = BoundObject.fwd_func_opts( "draw_string",
        {"string", "color", "origin", "align", "shadow_color", "shadow_offset"},
        {align = "left", color = {1, 1, 1, 1}, origin = {0, 0}}),

    --- Draw a @{sprite|Sprite} to `Overlay`.
    --
    -- This function is called with an "`opts`" parameter table. The parameters
    -- do not have to be in order.
    --
    -- **Valid Options:**
    --
    -- * `sprite` = The @{sprite|Sprite} to draw. *(required)*
    --
    -- * `color` = `{r, g, b, a}` vector representing the background color.
    -- Default `{0, 0, 0, 0}` (clear).
    --
    -- * `center` = `{x, y}` point relative to the screen center or entity edge
    -- in pixels. Default `{0, 0}`
    --
    -- * `rotation` = The angle of the sprite in **degrees**. Default `0`
    --
    -- * `scale` = Either a number or an `{x, y}` vector representing a scale
    -- factor. Default `1`
    --
    -- @function overlay:draw_sprite
    -- @tparam table opts The named parameters.
    -- @treturn nil
    --
    -- @usage
    -- local mysprite = MySprite:new()
    -- overlay:draw_sprite({
    --     sprite = mysprite,
    --     center = {0, -100},
    --     rotation = 180,
    --     scale = {2, 1}
    -- })
    draw_sprite = BoundObject.fwd_func_opts( "draw_sprite",
        {"sprite", "color", "center", "rotation", "scale"},
        {color = {0, 0, 0, 0}, center = {0, 0}, rotation = 0, scale = {1, 1}}),

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks


    --- Called once each frame during the Scene's update cycle. Modify values to
    -- display in this hook.
    --
    -- @tparam Overlay self The `Overlay` instance
    update = function(self) end,

    --- Called once each frame during the Scene's render cycle. Perform draw
    -- calls in this hook.
    --
    -- @tparam Overlay self The `Overlay` instance
    render = function(self) end,

})
