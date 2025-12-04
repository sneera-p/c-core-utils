local fs = require "lfs"

local local_include_pattern = '#include%s+"(.-%.h)"'
local system_include_pattern = '^#include%s+<(.-%.h)>'

local function find_headers(dir, headers)
   headers = headers or {}
   for file in lfs.dir(dir) do
      if file ~= "." and file ~= ".." then
         local path = dir .. "/" .. file
         local attr = lfs.attributes(path)
         if attr.mode == "directory" then
            find_headers(path, headers)
         elseif file:match("%.h$") then
            table.insert(headers, path)
         end
      end
   end
   return headers
end

local function search_local_includes(dir, file_path)
   local includes = {}
   for line in io.lines(file_path) do
      local header = line:match(local_include_pattern)
      if header then
         table.insert(includes, dir .. "/" .. header)
      end
   end
   return includes
end

local function contains_all(array, values)
   local lookup = {}
   for _, v in ipairs(array) do
      lookup[v] = true
   end
   for _, v in ipairs(values) do
      if not lookup[v] then
         return false
      end
   end
   return true
end

local function find_insert_order(dir)
   local headers = find_headers(dir)
   local includes = {}
   local order = {}
   for i = #headers, 1, -1 do
      local h = headers[i]
      local incl = search_local_includes(dir, h)
      if #incl == 0 then
         table.insert(order, h)
         table.remove(headers, i)
      else
         includes[h] = incl
      end
   end
   while #headers > 0 do
      local i = 1
      for i = #headers, 1, -1 do
         local h = headers[i]
         if contains_all(order, includes[h]) then
            table.insert(order, h)
            table.remove(headers, i)
         else
            i = i + 1
         end
      end
   end
   return order
end

local function generate_output(name, order)
   header_def = "__" .. string.upper(name) .. "_GENERATED_H"
   ins = {}
   out = {}
   table.insert(ins, "#ifndef " .. header_def .. "\n")
   table.insert(ins, "#define " .. header_def .. "\n\n")
   for _, file in ipairs(order) do
      table.insert(out, "/* <-- Begin " .. file .. " --> */\n\n")
      for line in io.lines(file) do
         if line:match(system_include_pattern) and not contains_all(ins, { line .. "\n"}) then
            table.insert(ins, line .. "\n")
         elseif not line:match(local_include_pattern) then
            table.insert(out, line .. "\n")
         end
      end
      table.insert(out, "\n/* <-- End " .. file .. " --> */\n\n\n")
   end
   table.insert(ins, "\n\n")
   table.insert(out, "#endif /* " .. header_def .. " */")
   return { ins, out }
end

local order = find_insert_order("./src")
local out = generate_output("ccoutils", order)
local f = assert(io.open("./build/ccoutils.h", "w"))
for _, chunk in ipairs(out) do
   for _, line in ipairs(chunk) do
      f:write(line)
   end
end
