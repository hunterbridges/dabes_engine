require 'dabes.overlay'

TestOverlay = Overlay:extend({
    font_name = "media/fonts/uni.ttf",
    font_size = 16,

    update = function(self)
    end,

    render = function(self)
        self:draw_string("Text drawn via a script!", {1, 1, 1, 1}, {0, 0}, "center")
    end
})
