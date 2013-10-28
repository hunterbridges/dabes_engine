require 'dabes.vector'

function isaabb(x)
    if type(x) ~= 'table' then return false end
    return x._isaabb == true
end

AABB = {
    origin = VPoint.new(0, 0),
    size = VPoint.new(0, 0),

    _isaabb = true,

    __sub = function(a, b)
        if not isvpoint(b) then
            error("Second operand must be a VPoint", 2)
        end

        return AABB.new(a.origin - b, a.size)
    end,

    __add = function(a, b)
        if not isvpoint(b) then
            error("Second operand must be a VPoint", 2)
        end

        return AABB.new(a.origin + b, a.size)
    end,

    __index = function(self, key)
        if key == 1 or key == 'x' then
            return self.origin.x
        elseif key == 2 or key == 'y' then
            return self.origin.y
        elseif key == 3 or key == 'w' then
            return self.size.x
        elseif key == 4 or key == 'h' then
            return self.size.y
        elseif key == 'tl' then
            return self.origin
        elseif key == 'tr' then
            return VPoint.new(self.origin.x + self.size.x,
                              self.origin.y)
        elseif key == 'bl' then
            return VPoint.new(self.origin.x,
                              self.origin.y + self.size.y)
        elseif key == 'br' then
            return VPoint.new(self.origin.x + self.size.x,
                              self.origin.y + self.size.y)
        end

        return rawget(AABB, key)
    end,

    __setindex = function(self, key, value)
        if key == 1 or key == 'x' then
            self.origin.x = value
        elseif key == 2 or key == 'y' then
            self.origin.y = value
        elseif key == 3 or key == 'w' then
            self.size.x = value
        elseif key == 4 or key == 'h' then
            self.size.y = value
        end
    end,

    contains = function(self, other)
        if isvpoint(other) then
            local br = self.br
            local tl = self.tl

            return (other.x <= br.x and
                    other.x >= tl.x and
                    other.y <= br.y and
                    other.y >= tl.y)
        elseif isaabb(other) then
            local s_br = self.br
            local s_tl = self.tl
            local o_br = other.br
            local o_tl = other.tl

            return (o_br.x <= s_br.x and
                    o_tl.x >= s_tl.x and
                    o_br.y <= s_br.y and
                    o_tl.y >= s_tl.y)
        else
            error("Second parameter must be VPoint or AABB", 2)
        end
    end,

    intersects = function(self, other)
        if isaabb(other) then
            local s_br = self.br
            local s_tl = self.tl
            local o_br = other.br
            local o_tl = other.tl

            return (s_tl.x < o_br.x and
                    s_br.x > o_tl.x and
                    s_tl.y < o_br.y and
                    s_br.y > o_tl.y)
        else
            error("Second parameter must be AABB", 2)
        end
    end,

    area = function(self)
        return self.size.x * self.size.y
    end,

    convert = function(self, other)
        if isvpoint(other) then
            return other - self.origin
        elseif isaabb(other) then
            return AABB.new(other.origin - self.origin, other.size)
        else
            error("Second parameter must be AABB or VPoint", 2)
        end
    end,

    convert_from = function(self, other)
        if isvpoint(other) then
            return other + self.origin
        elseif isaabb(other) then
            return AABB.new(other.origin + self.origin, other.size)
        else
            error("Second parameter must be AABB or VPoint", 2)
        end
    end
}

AABB.new = function(x, y, w, h)
    local instance = {}
    if isvpoint(x) and isvpoint(y) then
        instance.origin = x
        instance.size = y
    elseif (type(x) == 'number' and
            type(y) == 'number' and
            type(w) == 'number' and
            type(h) == 'number') then
        instance.origin = VPoint.new(x, y)
        instance.size = VPoint.new(w, h)
    else
        error("Must provide origin and size VPoints or xywh numbers", 2)
    end

    setmetatable(instance, AABB)
    return instance
end
