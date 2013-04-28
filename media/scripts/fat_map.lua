require "bindings"

fat_map = {
    map = "media/tilemaps/fat.tmx",
    music = {
      intro = "media/music/Climb.aif",
      loop = "media/music/Climb_loop.aif"
    }
}

function fat_map.configure(space_width, space_height)
    entities = {}

    parallax = fat_map._icecap_parallax()

    num_boxes = 100
    xo = 6.0
    for i = 1, num_boxes do
        entity = EntityConfig.create()

        entity.w = i / num_boxes * 4 + 1
        entity.h = i / num_boxes * 4 + 1

        entity.x = xo
        xo = xo + entity.w + 1

        entity.y = space_height - 8.0
        entity.rotation = math.pi / 16.0 * (i % 8)
        entity.mass = 100.0 + 900.0 * i / num_boxes
        entity.alpha = i / num_boxes * 1.0

        table.insert(entities, entity)
    end

    -- Player
    entities[1].w = 1.0
    entities[1].h = 1.0
    entities[1].sprite.texture = "media/sprites/megaman_run.png"
    entities[1].current_frame = 2;

    return parallax, unpack(entities)
end

-- Private
function fat_map._icecap_parallax()
    parallax = Parallax.create()

    parallax.sky_color.r = 0.0
    parallax.sky_color.g = 0.0
    parallax.sky_color.b = 0.698
    parallax.sea_color.r = 0.851
    parallax.sea_color.g = 0.851
    parallax.sea_color.b = 0.851
    parallax.y_wiggle = -20
    parallax.sea_level = 1.0

    yo = 80

    m_far = ParallaxLayer.create()
    m_far.p_factor = 0.2
    m_far.offset_y = yo - 103 - 102
    m_far.texture = "media/bgs/icecap_mountains_far.png"
    Parallax.add_layer(parallax, m_far)

    m_close = ParallaxLayer.create()
    m_close.p_factor = 0.2
    m_close.offset_y = yo - 102
    m_close.texture = "media/bgs/icecap_mountains_close.png"
    Parallax.add_layer(parallax, m_close)

    c_far = ParallaxLayer.create()
    c_far.p_factor = 0.3
    c_far.offset_y = yo
    c_far.texture = "media/bgs/icecap_clouds_far.png"
    Parallax.add_layer(parallax, c_far)

    c_mid = ParallaxLayer.create()
    c_mid.p_factor = 0.45
    c_mid.offset_y = yo + 25
    c_mid.texture = "media/bgs/icecap_clouds_mid.png"
    c_mid.y_wiggle = 25
    Parallax.add_layer(parallax, c_mid)

    c_close = ParallaxLayer.create()
    c_close.p_factor = 0.60
    c_close.offset_y = yo + 25 + 25
    c_close.texture = "media/bgs/icecap_clouds_close.png"
    c_close.y_wiggle = 25 + 25
    Parallax.add_layer(parallax, c_close)

    return parallax
end

