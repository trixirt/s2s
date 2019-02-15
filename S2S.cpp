//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright Tom Rix 2019, all rights reserved.
//
//===----------------------------------------------------------------------===//
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
#else
#include <malloc.h> // for _alloca used by boost
#endif
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "Configuration.h"
#include "Process.h"
#include "Scripting.h"
#include "TempFile.h"

#include "clang/Tooling/CompilationDatabase.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace std;
using namespace clang::tooling;
using namespace boost::filesystem;

static cl::opt<bool> Help("h", cl::desc("Alias for -help"), cl::Hidden);

static cl::opt<string> Script("script");
static cl::opt<string> Filter("filter");
static cl::opt<string> DB("db");

void scrub_cl(vector<string> &CL, string &D, string &FD, string &F) {
  if (CL.begin() != CL.end())
    CL.erase(CL.begin());

  auto i = find(CL.begin(), CL.end(), F);
  if (i != CL.end()) {
    CL.erase(i);
  }
  i = find(CL.begin(), CL.end(), "-c");
  if (i != CL.end()) {
    CL.erase(i);
  }
  i = find(CL.begin(), CL.end(), "-o");
  if (i != CL.end()) {
    auto j = next(i, 1);
    if (j != CL.end())
      CL.erase(j);
    i = find(CL.begin(), CL.end(), "-o");
    if (i != CL.end()) {
      CL.erase(i);
    }
  }
  //
  // Because the source is moving, add an include path
  // back to the original source
  string sd = "-I";
  string s = sd + FD;
  CL.insert(CL.begin(), s);
  for (auto cl : CL) {
    if (boost::algorithm::starts_with(cl, sd)) {
      s = cl.substr(2, string::npos);
      path f = s;
      if (f.is_relative()) {
        path p = D / f;
        s = p.string();
        s = "-I" + s;
        replace(CL.begin(), CL.end(), cl, s);
      }
    }
  }
}

int main(int argc, char **argv) {
  int Ret = 1;
  cl::ParseCommandLineOptions(argc, argv);
  lua_init();
  if (Filter != "")
    lua_file(Filter.c_str());
  if (Script != "")
    lua_file(Script.c_str());

  std::vector<std::string> failures, successes;
  std::unique_ptr<CompilationDatabase> Compilations;

  if (DB != "") {
    string err;
    Compilations = CompilationDatabase::loadFromDirectory(DB, err);
  }

  for (auto CC : Compilations->getAllCompileCommands()) {
    string Exe = CC.CommandLine[0];
    if (Filter != "")
      if (!FilterDBEntry(CC.CommandLine, CC.Filename, CC.Directory, Exe))
        continue;

    path f = CC.Filename;
    path d = CC.Directory;
    path p;

    if (f.is_absolute())
      p = f;
    else
      p = boost::filesystem::absolute(f, d);
    file_status stat(boost::filesystem::status(p));
    string File = p.string();

    if (stat.type() == boost::filesystem::file_not_found) {
      fprintf(stderr, "\nCould not find file : %s\n", File.c_str());
      fflush(stderr);
      continue;
    }

    fprintf(stdout, "\nCurrent file : %s\n", File.c_str());
    fflush(stdout);

    string Ext = boost::filesystem::extension(File);
    string FileDirectory = p.parent_path().string();

    scrub_cl(CC.CommandLine, CC.Directory, FileDirectory, CC.Filename);

    vector<string> EditorCL, S2SCL, ICL;
    for (auto c : CC.CommandLine)
      ICL.push_back(c);

    int Result;
    bool editorOk = false;
    string FileCopy;
    TempFileCopy(FileCopy, File, Ext);
    if (FileCopy.size()) {
      string dummy;
      string editorExt, s2sExt;
      if (GetS2SExtension(s2sExt)) {
        if (s2sExt == "stderr" || s2sExt == "stdout") {
          string s2sStdout, s2sStderr;
          if (GetS2SCommandLine(S2SCL, ICL, FileCopy, dummy, Exe)) {
            Result = Process(S2SCL, dummy, s2sStdout, s2sStderr);
            if (IsS2SOk(Result)) {
              if (GetEditorExtension(editorExt)) {
                if (editorExt == "stdin") {
                  string editorStdin = s2sStdout;
                  if (s2sExt == "stderr")
                    editorStdin = s2sStderr;
                  if (GetEditorCommandLine(EditorCL, ICL, dummy, FileCopy,
                                           Exe)) {
                    Result = Process(EditorCL, editorStdin, dummy, dummy);
                    if (IsEditorOk(Result)) {
                      editorOk = true;
                      // yeah
                    }
                  }
                } else {
                }
              }
              // yeah!
            }
            TempFileRemove(s2sStdout);
            TempFileRemove(s2sStderr);
          }
        } else {
          string s2sOut;
          TempFileName(s2sExt, s2sOut);
          if (s2sOut.size()) {
            if (GetS2SCommandLine(S2SCL, ICL, FileCopy, s2sOut, Exe)) {
              Result = Process(S2SCL);
              if (IsS2SOk(Result)) {
                string editorExt;
                if (GetEditorExtension(editorExt)) {
                  if (editorExt == "stdin") {
                    string editorStdin = s2sOut;
                    if (GetEditorCommandLine(EditorCL, ICL, dummy, FileCopy,
                                             Exe)) {
                      Result = Process(EditorCL, editorStdin, dummy, dummy);
                      if (IsEditorOk(Result)) {
                        editorOk = true;
                      }
                    }
                  } else {
                    if (GetEditorCommandLine(EditorCL, ICL, s2sOut, FileCopy,
                                             Exe)) {
                      Result = Process(EditorCL);
                      if (IsEditorOk(Result)) {
                        editorOk = true;
                      }
                    }
                  }
                }
              }
            }
            TempFileRemove(s2sOut);
          }
        }
      }
    }

    bool testOk = false;
    if (editorOk) {
      vector<string> TC;
      string sext = boost::filesystem::extension(p);
      testOk = true;
      if (GetTestConfigurations(TC, Exe, sext)) {
        for (auto tc : TC) {
          vector<string> TS;
          if (GetTestStages(TS, tc)) {
            string IF = FileCopy;
            for (auto ts : TS) {
              string ext;
              GetTestExtension(ext, ts);
              string tf;
              TempFileName(ext, tf);
              if (tf.size()) {
                string OF = tf;
                vector<string> OCL;
                for (auto c : CC.CommandLine)
                  ICL.push_back(c);
                if (GetTestCommandLine(OCL, ICL, tc, ts, IF, OF))
                  Result = Process(OCL);
                if (!IsTestOk(Result, ts)) {
                  fprintf(stderr, "\nFAILED %s\n", File.c_str());
                  fflush(stderr);
                  testOk = false;
                }
                if (IF != FileCopy)
                  TempFileRemove(IF);
                IF = OF;
              }
            }
          }
        }
      }
    }

    if (testOk) {
      vector<string> DiffCL;
      if (GetDiffCommandLine(DiffCL, File, FileCopy)) {
        Result = Process(DiffCL);
        if (IsDiffOk(Result)) {
          if (IsOverWriteOk()) {
            TempFileOverWrite(File, FileCopy);
          }
        }
      }
      successes.push_back(File);
    } else {
      failures.push_back(File);
    }
    // Stop at 1
    // break;
  }
  Ret = 0;

  if (Script != "") {
    lua_cleanup();
  }

  if (failures.size() + successes.size()) {
    fprintf(stdout, "\nFailures\n");
    for (auto f : failures) {
      fprintf(stdout, "%s\n", f.c_str());
    }

    fprintf(stdout, "\nSuccesses\n");
    for (auto f : successes) {
      fprintf(stdout, "%s\n", f.c_str());
    }

    double n = successes.size();
    double d = successes.size() + failures.size();
    double p = (100.0 * n) / d;

    fprintf(stdout, "Success rate %f\n", p);
  } else {
    fprintf(stdout, "No work done\n");
  }
  fflush(stdout);

  return Ret;
}
