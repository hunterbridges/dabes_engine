-- Object
--
-- Some syntax sugar to inherit behavior.

Object = {
    extend = function(super, sub)
        setmetatable(sub, super)
        sub.__index = sub

        return sub
    end
}
Object.__index = Object
