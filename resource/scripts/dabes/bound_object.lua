--- Some weird meta stuff that lets us create a nice object oriented interface
-- around the C bindings. This is mostly used internally, but the `init`
-- hook is useful for custom objects.
--
--  @{bound_object|BoundObject} extends @{object|Object}
--
-- @module bound_object
-- @type BoundObject

require 'dabes.object'

BoundObject = Object:extend({

--- Configuration.
-- Required by subclass declarations. Used when instantiating
-- concrete subclasses.
-- @section configuration

    --- The Lua library provided by the game engine that instance method
    -- calls are forwarded to.
    lib = nil,

--- Properties.
-- Significant fields on an instance.
-- @section properties

    --- The userdata that represents an instance inside of the game
    -- engine.
    real = nil,

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new instance of `BoundObject`.
    --
    -- This is the external method used to create a bound object.
    -- It calls the `realize` hook, then the `init` hook.
    --
    -- `new` should not be overloaded by a subclass. Implement the `init`
    -- hook if you want to perform actions when the object is created.
    --
    -- @name BoundObject:new
    -- @param ... Constructor parameters. These are sent to `realize` and `init`
    -- @treturn BoundObject A new instance of `BoundObject`
    new = function(class, ...)
        local bound = Object:new()
        local meta = getmetatable(bound)
        setmetatable(meta, class)

        bound.real = bound:realize(...)
        dab_registerinstance(bound.real, bound)
        bound.born_at = dab_engine.ticks()
        bound:init(...)

        return bound
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Responsible for interacting with the `lib` and returning a userdata.
    --
    -- This hook is **required**. A `BoundObject` will not function without
    -- an implementation of `realize`.
    --
    -- @name realize
    -- @tparam BoundObject self An instance in the throws of instantiation
    -- @param ... Any arguments passed to @{BoundObject:new|new}
    -- @treturn userdata The userdata that will be set to `real`
    realize = function(self, ...) end,

    --- Called immediately after `realize`.
    --
    -- This is only called once in the `BoundObject`'s lifecycle.
    --
    -- @tparam BoundObject self The realized instance
    -- @param ... Any arguments passed to @{BoundObject:new|new}
    init = function(self, ...) end,

--- Helpers.
-- Utility functions stored on a class. Called using dot syntax.
-- e.g. `Class.helper("foo")`
-- @section helpers

    --- Generate a method that performs one-to-one argument forwarding
    -- to `lib`
    --
    -- @name BoundObject.fwd_func
    -- @tparam string name The name of the `lib` function
    -- @treturn function
    fwd_func = function(name)
        return function(self, ...)
            if not self.real then return nil end

            return self.real[name](self.real, ...)
        end
    end,

    --- Generate a method that performs one-to-one argument forwarding,
    -- replacing each argument with `arg.real`
    --
    -- @name BoundObject.fwd_func_real
    -- @tparam string name The name of the `lib` function
    -- @treturn function
    fwd_func_real = function(name)
        return function(self, ...)
            if not self.real then return nil end

            return self.real[name](self.real, map_real(...))
        end
    end,

    --- Generate a method that performs key-mapped argument forwarding.
    --
    -- This will generate a method that accepts one argument, an `opts`
    -- table.
    --
    -- The values of the `opts` table are sent to the `lib` function specified
    -- by `name`. They are arranged in the order that the table keys
    -- appear in the `keys` list. Any keys not specified in the `keys` list
    -- are ignored.
    --
    -- @name BoundObject.fwd_func_opts
    -- @tparam string name The name of the `lib` function
    -- @tparam table keys The ordered list of keys to be mapped to the `lib`
    -- function
    -- @tparam table defaults An optional defaults table
    -- @treturn function
    fwd_func_opts = function(name, keys, defaults)
        return function(self, opts)
            if not self.real then return nil end
            local fwded = self.real[name]

            local args = nil
            if defaults == nil then
                args = copy(opts)
            else
                args = merge(defaults, opts)
            end

            local ordered = {}
            for i, v in ipairs(keys) do
                ordered[i] = args[v]
            end

            return fwded(self.real, unpack(ordered))
        end
    end,

    --- Generate a `lib` forwarded method intended to add an object to a
    -- collection.
    --
    -- This performs some caching in the Lua VM so added objects don't get
    -- collected by the GC.
    -- @name BoundObject.fwd_adder
    -- @tparam string name The name of the `lib` function
    -- @treturn function
    fwd_adder = function(name)
        return function(self, member, ...)
            if not self.real then return nil end
            local fwded = self.real[name]

            local cachekey = '_'..name..'_cache'
            if self[cachekey] == nil then rawset(self, cachekey, {}) end
            -- Hold on to the member so it isn't collected
            self[cachekey][member] = true

            return fwded(self.real, member, ...)
        end
    end,

    --- Generate a `lib` forwarded method intended to remove an object from a
    -- collection.
    --
    -- This performs some caching in the Lua VM so removed objects can get
    -- collected by the GC.
    -- @name BoundObject.fwd_remover
    -- @tparam string name The name of the `lib` function
    -- @treturn function
    fwd_remover = function(name)
        return function(self, member, ...)
            if not self.real then return nil end
            local fwded = self.real[name]

            local cachekey = '_'..name..'_cache'
            if self[cachekey] == nil then rawset(self, cachekey, {}) end
            self[cachekey][member] = nil

            return fwded(self.real, member, ...)
        end
    end,

    --- Generate a `lib` forwarded setter that enables the corresponding
    -- property to be set to `nil` by the game engine.
    --
    -- This implements an ad-hoc hook called `_zero` which is called by the
    -- engine when the object has been destroyed. An example of this behavior
    -- is how a ${music|Music} instance is destroyed when it is finished
    -- playing.
    --
    -- @name BoundObject.fwd_zeroing_setter
    -- @tparam string fname The name of the `lib` function
    -- @tparam string pname The name of the corresponding property
    -- @treturn function
    fwd_zeroing_setter = function(fname, pname)
        return function(self, member, ...)
            if not self.real then return nil end
            local fwded = self.real[fname]

            if member ~= nil then
                if member._zeroable == nil then
                    member._zeroable = {}
                end

                local lookup = {reffer = self, refname = pname}
                table.insert(member._zeroable, lookup)

                member._zero = function(mself)
                    for i, v in ipairs(mself._zeroable) do
                        local lookup = v
                        print("Zeroing", lookup.reffer, " -> ", lookup.refname)
                        lookup.reffer[lookup.refname] = nil
                    end
                    mself._zeroable = nil
                end
            end

            return fwded(self.real, member, ...)
        end
    end,

    --- A function that throws a "read only" error, intended for
    -- setter forwarding.
    --
    -- @name BoundObject.readonly
    -- @treturn function
    readonly = function(self, key, val)
        error("Attempting to set a read only property", 3)
    end

})

