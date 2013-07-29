--- An interface for managing groups of objects that belong to a parent.
--
--  @{collection|Collection} extends @{object|Object}
--
-- @module collection
-- @type Collection

require 'dabes.object'

Collection = Object:extend({

--- Properties.
-- Significant fields on an instance.
-- @section properties

    --- The parent object of the collection.
    owner = nil,

--- Class Methods.
-- Must be called on `Class`, with a capital leading character.
-- e.g. `Class:method("foo")`
-- @section classmethods

    --- Create a new `Collection`
    -- @function Collection:new
    -- @tparam Object owner The `owner`
    -- @treturn Collection
    new = function(class, owner)
        local instance = Object:new()
        local meta = getmetatable(instance)
        setmetatable(meta, class)

        instance.owner = owner

        return instance
    end,

--- Instance Methods.
-- Must be called on an instance of `Class`.
-- e.g. `instance:method("foo")`
-- @section instancemethods

    --- Add a member to the `Collection`. This caches the member
    -- so it doesn't get garbage collected prematurely. It also calls the
    -- `adder` hook with any variable arguments.
    --
    -- @function collection:add
    -- @param member The member to add to the collection
    -- @param ... Arguments to be passed to `adder`
    -- @treturn Whatever `adder` returns
    add = function(self, member, ...)
        self._cache[member] = true
        return self.adder(self.owner, member, ...)
    end,

    --- Remove a member from the `Collection`. This removes the
    -- member from the cache so it can get garbage collected. It also calls
    -- the `remove` hook with any variable arguments.
    --
    -- @function collection:remove
    -- @param member The member to remove the collection
    -- @param ... Arguments to be passed to `remover`
    -- @treturn Whatever `remover` returns
    remove = function(self, member, ...)
        local rc = nil
        rc = self.remover(self.owner, member, ...)
        self._cache[member] = nil
        return rc
    end,

--- Hooks.
-- Callbacks implemented in subclasses to customize behavior. Hooks are called
-- on individual instances.
-- @section hooks

    --- Called when a member is added to the collection.
    --
    -- @param owner The owner of the collection
    -- @param member The member being added
    -- @param ... Any other arguments passed to `collection:add`
    adder = function(owner, member, ...) end,

    --- Called when a member is removed from the collection.
    --
    -- @param owner The owner of the collection
    -- @param member The member being removed
    -- @param ... Any other arguments passed to `collection:remove`
    remover = function(owner, member, ...) end,

})
