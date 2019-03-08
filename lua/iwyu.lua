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
function GetTestConfigurations(Exe, Ext)
  local r = { "default" }
  return r
end

function GetTestStages(TestConfiguration)
  local r = {}
  if TestConfiguration == "default" then
    r[#r+1] = "default"
  end
  return r
end

function GetTestCommandLine(CommandLine, TestConfiguration, TestStage, InputFile, OutputFile, Exe)
  local r = {}

  if TestConfiguration == "default" then
    r[#r+1] = Exe
  end

  for k, v in pairs(CommandLine) do
    r[#r+1] = v
  end

  r[#r+1] = "-fsyntax-only"
  r[#r+1] = InputFile	
  r[#r+1] = "-o"
  r[#r+1] = OutputFile

  return r
end

function GetTestExtension(TestStage)
  local r = ".default"
  return r
end

function IsTestOk(I, TS)
  local r = 0
  if I == 0 then
    r = 1
  end
  return r
end

function GetS2SExtension()
  -- stdout
  -- local r = "stdout"
  local r = "stderr"
  return r
end

function isGccOption(option)
  local r = false
  if option == "-Wshadow=local" then
    r = true
  end
  return r
end

function GetS2SCommandLine(CommandLine, InputFile, OutputFile, Exe)
  local r = {}
  r[#r+1] = "include-what-you-use"
  r[#r+1] = "-Xiwyu"
  r[#r+1] = "--verbose=3"
  --
  -- Keep list
  -- This is a list of headers that iwyu deletes but are really needed
  local keepList = {
    -- Add your headers as strings here
  }
  for _, v in ipairs(keepList) do
    r[#r+1] = "-Xiwyu"
    r[#r+1] = "--keep=" .. v
  end

  r[#r+1] = "-x"
  -- check if the extension is cpp or cc
  local ext = string.match(InputFile, '[^.]+$')
  if ext == "cpp" or ext == "cc" then
    r[#r+1] = "c++"
  else
    r[#r+1] = "c"
  end
  for k, v in pairs(CommandLine) do
    if isGccOption(v) then
    -- clang based tools throw spurious warning when none clang options
    -- are seen, so remove them from the command line
    else
      r[#r+1] = v
    end
  end
  r[#r+1] = InputFile	
  return r
end

function IsS2SOk(I)
  local r = 0
  if I > 1 and I < 127 then
    r = 1
  end
  return r
end

function GetEditorExtension()
  local r = "stdin"
  return r
end

function GetEditorCommandLine(CommandLine, InputFile, OutputFile, Exe)
  local r = {}
  r[#r+1] = "fix_includes.py"
  r[#r+1] = "--nocomments"
  return r
end

function IsEditorOk(I)
  local r = 0
  if I > 0 then
    r = 1
  end
  return r
end

function IsOverWriteOk()
  local r = 0
  return r
end

function GetDiffCommandLine(AFile, BFile)
  local r = {}
  r[#r+1] = "diff"
  r[#r+1] = "-up"
  r[#r+1] = AFile
  r[#r+1] = BFile
  return r
end

function IsDiffOk(I)
  local r = 0
  if I == 1 then
    r = 1
  end
  return r
end
