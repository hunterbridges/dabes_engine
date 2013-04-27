EntityConfig = {
    identifier = "entity_config"
}
EntityConfig.__index = EntityConfig

function EntityConfig.create()
    local entity = {}
    setmetatable(entity, EntityConfig)
    return entity
end


Parallax = {
    identifier = "parallax",

    sky_color = { r = 0.0, g = 0.0, b = 1.0, a = 1.0 },
    sea_color = { r = 0.0, g = 1.0, b = 0.0, a = 1.0 },
    y_wiggle = 0.0,
    sea_level = 0.5,

    num_layers = 0,
    layers = {}
}
Parallax.__index = Parallax

function Parallax.create()
    local p = {}
    setmetatable(p, Parallax)
    return p
end

function Parallax.add_layer(p, p_layer)
    table.insert(p.layers, p_layer)
    p.num_layers = p.num_layers + 1
end


ParallaxLayer = {
    identifier = "parallax_layer",
    p_factor = 1.0,
    texture = "",
    offset_x = 0.0,
    offset_y = 0.0,
    base_scale = 1.0,
    y_wiggle = 0.0
}
ParallaxLayer.__index = ParallaxLayer

function ParallaxLayer.create()
    local p_layer = {}
    setmetatable(p_layer, ParallaxLayer)
    return p_layer
end
