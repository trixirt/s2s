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
  local r = "stdout"
  return r
end

function GetS2SCommandLine(CommandLine, InputFile, OutputFile, Exe)
  local r = {}
  r[#r+1] = "true"
  return r
end

function IsS2SOk(I)
  local r = 1
  return r
end

function GetEditorExtension()
  local r = "stdin"
  return r
end

function GetEditorCommandLine(CommandLine, InputFile, OutputFile, Exe)
  local r = {}
  r[#r+1] = "true"
  return r
end

function IsEditorOk(I)
  local r = 1
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
