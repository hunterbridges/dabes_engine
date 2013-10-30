require 'table'
require 'math'

function isvpoint(x)
    if type(x) ~= 'table' then return false end
    return x._isvpoint == true
end

VPoint = {
    _isvpoint = true,

    __tostring = function(a)
        return "<"..a.x..", "..a.y..">"
    end,

    __unm = function(a)
        return VPoint.new(-a.x, -a.y)
    end,

    __add = function(a, b)
        if not isvpoint(a) or not isvpoint(b) then
            error("Both operands must be a VPoint", 2)
        end

        return VPoint.new(a.x + b.x, a.y + b.y)
    end,

    __sub = function(a, b)
        if not isvpoint(a) or not isvpoint(b) then
            error("Both operands must be a VPoint", 2)
        end

        return VPoint.new(a.x - b.x, a.y - b.y)
    end,

    __mul = function(a, b)
        if isvpoint(a) and isvpoint(b) then
            return VPoint.new(a.x * b.x, a.y * b.y)
        elseif isvpoint(a) and type(b) == 'number' then
            return VPoint.new(a.x * b, a.y * b)
        end
    end,

    __div = function(a, b)
        if isvpoint(a) and isvpoint(b) then
            error("Right operand must not be a VPoint", 2)
        elseif isvpoint(a) and type(b) == 'number' then
            return VPoint.new(a.x / b, a.y / b)
        end
    end,

    __index = function(self, key)
        if key == 1 or key == 'x' then
            return rawget(self, 1)
        elseif key == 2 or key == 'y' then
            return rawget(self, 2)
        end
        return rawget(VPoint, key)
    end,

    __setindex = function(self, key, value)
        if key == 1 or key == 'x' then
            return rawset(self, 1, value)
        elseif key == 2 or key == 'y' then
            return rawset(self, 2, value)
        end
    end,

    __eq = function(a, b)
        if not isvpoint(b) then
            return false
        end
        return a.x == b.x and a.y == b.y
    end,

    mag = function(self)
        return math.sqrt(math.pow(self.x, 2) + math.pow(self.y, 2))
    end
}

VPoint.new = function(x, y)
    local instance = {}
    if type(x) == 'number' and type(y) == 'number' then
        instance[1] = x
        instance[2] = y
    elseif type(x) == 'table' then
        instance[1] = x[1]
        instance[2] = x[2]
    end
    setmetatable(instance, VPoint)
    return instance
end

