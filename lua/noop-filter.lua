function FilterDBEntry(CommandLine, InputFile, InputDirectory, Exe)
  local r = 1
  --
  -- Do not run on these files
  local norunList = {
  }
  for _, v in ipairs(norunList) do
    if InputFile == v then
      return 0
    end
  end
  -- Customize filter here
  --
  -- To look at a single file, uncomment
  -- local f = ""
  -- r = 0
  -- if InputFile == f then
  --  r = 1
  -- end
  return r
end
