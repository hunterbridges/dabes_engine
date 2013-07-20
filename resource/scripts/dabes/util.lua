--- Assorted utility functions

--- Creates a new weak-keyed table
-- @return A new weak-keyed table
function newweaktable()
  local newtable = {}
  setmetatable(newtable, {__mode = "k"})
  return newtable
end

--- Copies a given table
-- This works by creating a new table, iterating ```t```,
-- and setting the key-value pairs in the new table.
-- This does not modify ```t```.
-- @tparam table t The table to be copied
-- @return The newly copied table, or ```nil``` if ```t``` is ```nil```
function copy(t)
    if t == nil then return nil end

    local t2 = {}
    for k,v in pairs(t) do
      t2[k] = v
    end
    return t2
end

--- Merges key-value pairs from table ```b``` onto a copy of table ```a```.
-- This does not modify ```a``` or ```b```.
-- @tparam table a The table to be copied and overwritten
-- @tparam table b The table with which to overwrite values
-- @return A new merged table
function merge(a, b)
    if a == nil then return copy(b) end
    if b == nil then return copy(a) end

    local merged = copy(a)
    for k,v in pairs(b) do
        merged[k] = v
    end
    return merged
end

--- Maps all bound object inputs to their corresponding userdata
-- @param ... Variable list of bound objects
-- @return Variable list of userdata
function map_real(...)
    local uds = {}
    for i = 1, select("#", ...) do
        uds[i] = select(i, ...).real
    end
    return unpack(uds)
end

