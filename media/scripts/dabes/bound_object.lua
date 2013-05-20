BoundObject = {
    binding = nil,  -- The binding library for this bound object.
    real = nil      -- The userdata for the C object this represents
}
BoundObject.__index = BoundObject

function BoundObject.new(klass, proto)
    proto.__index = proto

    local bound = {}
    setmetatable(proto, klass)
    setmetatable(bound, proto)

    bound.real = bound.binding.new(bound.i_params)
    if bound.init then
        bound:init()
    end

    return bound
end

function BoundObject.extend(obj)
    setmetatable(obj, BoundObject)
    obj.__index = obj

    return obj
end
