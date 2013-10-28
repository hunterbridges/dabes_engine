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
