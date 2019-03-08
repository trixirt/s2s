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
  local r = { true }
  if Exe == "cc" then
    r = {"gcc" }
  elseif Exe == "c++" then
    r = {"g++" }
  end
  return r
end

function GetTestStages(TestConfiguration)
  local r = {}
  if TestConfiguration == "gcc" then
    r[#r+1] = "cc"
  elseif TestConfiguration == "g++" then
    -- r[#r+1] = "cxxpp"
    r[#r+1] = "cxx"
  elseif TestConfiguration == "clang" then
    r[#r+1] = "cc"
  elseif TestConfiguration == "clang++" then
    r[#r+1] = "cxx"
  end
  return r
end

function isCCTest(config, stage)
  local r = false;
  if stage == "cc" or stage == "cpp" then
    r = true
  end
  return r
end

function isCxxOption(option)
  local r = false
  if option == "-std=gnu++98" or
     option == "-Woverloaded-virtual" or
     option == "-fno-rtti" then
    r = true
  end
  return r
end

function isGccOption(option)
  local r = false
  if option == "-Wshadow=local" then
    r = true
  end
  return r
end

function GetTestCommandLine(CommandLine, TestConfiguration, TestStage, InputFile, OutputFile)
  local r = {}

  if TestConfiguration == "gcc" then
    if TestStage == "cpp" then
      r[#r+1] = "cpp"
    elseif TestStage == "cc" then
      r[#r+1] = "gcc"
    end
  elseif TestConfiguration == "g++" then
    if TestStage == "cxxpp" then
      r[#r+1] = "cpp"
    elseif TestStage == "cxx" then
      r[#r+1] = "g++"
    end    
  elseif TestConfiguration == "clang" then
    if TestStage == "cc" then
      r[#r+1] = "clang"
      r[#r+1] = "-x"
      r[#r+1] = "c"
    end
  elseif TestConfiguration == "clang++" then
    if TestStage == "cxx" then
      r[#r+1] = "clang++"
      r[#r+1] = "-x"
      r[#r+1] = "c++"
    end    
  end

  for k, v in pairs(CommandLine) do
    if isCCTest(TestConfiguraton, TestStage) and isCxxOption(v) then
      -- clang is not tolerant of c++ options
      -- so remove them from the command line
    else
      r[#r+1] = v
    end
  end

  if TestStage == "cc" or TestStage == "cxx" then
    r[#r+1] = "-fsyntax-only"
  end
  r[#r+1] = InputFile	
  r[#r+1] = "-o"
  r[#r+1] = OutputFile

  return r
end

function GetTestExtension(TestStage)
  local r = ""
  if TestStage == "cpp" then
    r = ".i"
  elseif TestStage == "cxxpp" then
    r = ".ii"
  elseif TestStage == "cc" then
    r = ".o"
  elseif TestStage == "cxx" then
    r = ".o"
  end
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
  local r = ".xml"
  return r
end

function GetS2SCommandLine(CommandLine, InputFile, OutputFile, Exe)
  local r = {}
  r[#r+1] = "include-what-you-use"
  r[#r+1] = "-Xiwyu"
  r[#r+1] = "--verbose=1"
  r[#r+1] = "-Xiwyu"
  r[#r+1] = "--no_reorder"
  r[#r+1] = "-Xiwyu"
  r[#r+1] = "--output_replacements_xml=" .. OutputFile
  --
  -- Keep list
  -- This is a list of headers that iwyu deletes but are really needed
  local keepList = {
    -- Add your headers as strings here
  }
  r[#r+1] = "-x"
  if Exe == "c++" then
    r[#r+1] = "c++"
  else
    r[#r+1] = "c"
  end
  for k, v in pairs(CommandLine) do
    if not isGccOption(v) then
       r[#r+1] = v
    end
  end
  r[#r+1] = InputFile	
  return r
end

function IsS2SOk(I)
  local r = 0
  if I > 1 then
    r = 1
  end
  return r
end

function GetEditorExtension()
  local r = "xml"
  return r
end

function GetEditorCommandLine(CommandLine, InputFile, OutputFile, Exe)
  local r = {}
  r[#r+1] = "noreoder_fix_includes.py"
  r[#r+1] = InputFile
  return r
end

function IsEditorOk(I)
  local r = 0
  if I == 0 then
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
