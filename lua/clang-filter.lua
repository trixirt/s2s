--===----------------------------------------------------------------------===
--
--                     The LLVM Compiler Infrastructure
--
-- This file is distributed under the University of Illinois Open Source
-- License. See LICENSE.TXT for details.
--
-- Copyright Tom Rix 2019, all rights reserved.
-- 
--===----------------------------------------------------------------------===
function FilterDBEntry(CommandLine, InputFile, InputDirectory, Exe)
  local r = 0
  local is_clang = string.match(Exe, 'clang')
  if is_clang == 'clang' then
    r = 1
  end
  return r
end
