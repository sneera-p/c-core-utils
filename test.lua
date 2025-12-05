local lfs = require "lfs"

local function find_src_files(dir, srcs)
   srcs = srcs or {}
   for file in lfs.dir(dir) do
      if file ~= "." and file ~= ".." then
         local path = dir .. "/" .. file
         local attr = lfs.attributes(path)
         if attr.mode == "directory" then
            find_src_files(path, srcs)
         elseif file:match("%.c$") then
            table.insert(srcs, path)
         end
      end
   end
   return srcs
end

local function run_cmd(cmd)
   local handle = io.popen(cmd .. " 2>&1", "r")
   local out = handle:read("*a")
   handle:close()
   return out
end

local function main()
   local out_file = "./build/test"
   local test_dir = arg[1]
   if not test_dir or test_dir == "" then
      print("Error: test directory not specified")
      return
   end
   local cmd = table.concat({
      "gcc",
      "-I./build",
      "-I" .. test_dir,
      "-o " .. out_file,
      table.concat(find_src_files(test_dir), " "),
      "-lcmocka"
   }, " ")
   compile = run_cmd(cmd)
   if compile == "" then
      print(run_cmd(out_file))
   else
      print(compile)
   end
end

main()
