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
static cl::opt<bool> Verbose("verbose");
static cl::opt<bool> SaveTemps("save-temps");

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
  bool FatalError = false;
  cl::ParseCommandLineOptions(argc, argv);
  lua_init();
  if (Filter != "")
    if (lua_file(Filter.c_str())) {
      FatalError = true;
      fprintf(stderr, "Fatal error in filter file %s\n", Filter.c_str());
      fflush(stderr);
    }
  if (Script != "")
    if (lua_file(Script.c_str())) {
      FatalError = true;
      fprintf(stderr, "Fatal error in script file %s\n", Script.c_str());
      fflush(stderr);
    }

  std::vector<std::string> failures, successes;
  std::unique_ptr<CompilationDatabase> Compilations;

  if (DB != "") {
    string Err;
    Compilations = CompilationDatabase::loadFromDirectory(DB, Err);
    if (Compilations == nullptr) {
      FatalError = true;
      fprintf(
          stderr,
          "Fatal error loading compile_command.json from from %s directory\n",
          DB.c_str());
      fprintf(stderr, "Load error : %s\n", Err.c_str());
      fflush(stderr);
    }
  }

  if (FatalError == true)
    goto bail;

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
            if (Verbose) {
              cout << "S2S Command line" << std::endl;
              for (auto s : S2SCL)
                cout << s << " ";
              cout << std::endl;
            }
            Result = Process(S2SCL, dummy, s2sStdout, s2sStderr);

            if (SaveTemps) {
              fprintf(stdout, "S2S input  temp file %s\n", FileCopy.c_str());
              fprintf(stdout, "S2S stdout temp file %s\n", s2sStdout.c_str());
              fprintf(stdout, "S2S stderr temp file %s\n", s2sStderr.c_str());
            }

            if (IsS2SOk(Result)) {
              if (GetEditorExtension(editorExt)) {
                if (editorExt == "stdin") {
                  string editorStdin = s2sStdout;
                  if (s2sExt == "stderr")
                    editorStdin = s2sStderr;
                  string editorStdout, editorStderr;
                  if (GetEditorCommandLine(EditorCL, ICL, dummy, FileCopy,
                                           Exe)) {
                    if (Verbose) {
                      cout << "Editor Command line" << std::endl;
                      for (auto s : EditorCL)
                        cout << s << " ";
                      cout << std::endl;
                    }
                    Result = Process(EditorCL, editorStdin, editorStdout,
                                     editorStderr);

                    if (SaveTemps) {
                      fprintf(stdout, "Editor input temp file %s\n",
                              FileCopy.c_str());
                      fprintf(stdout, "Editor stdin temp file %s\n",
                              editorStdin.c_str());
                      fprintf(stdout, "Editor stdout temp file %s\n",
                              editorStdout.c_str());
                      fprintf(stdout, "Editor stderr temp file %s\n",
                              editorStderr.c_str());
                    } else {
                      TempFileRemove(editorStdout);
                      TempFileRemove(editorStderr);
                    }

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
            if (!SaveTemps) {
              TempFileRemove(s2sStdout);
              TempFileRemove(s2sStderr);
            }
          }
        } else {
          string s2sOut;
          TempFileName(s2sExt, s2sOut);
          if (s2sOut.size()) {
            if (GetS2SCommandLine(S2SCL, ICL, FileCopy, s2sOut, Exe)) {
              if (Verbose) {
                cout << "S2S Command line" << std::endl;
                for (auto s : S2SCL)
                  cout << s << " ";
                cout << std::endl;
              }
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
            if (!SaveTemps) {
              TempFileRemove(s2sOut);
            } else {
              fprintf(stdout, "S2S output temp file %s\n", s2sOut.c_str());
            }
          }
        }
      }
    }

    bool testOk = false;
    if (editorOk) {
      vector<string> TC;
      string Ext = boost::filesystem::extension(p);
      testOk = true;
      if (GetTestConfigurations(TC, Exe, Ext)) {
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
                if (GetTestCommandLine(OCL, ICL, tc, ts, IF, OF)) {
                  if (Verbose) {
                    cout << "Test Command line" << std::endl;
                    for (auto s : OCL)
                      cout << s << " ";
                    cout << std::endl;
                  }

                  Result = Process(OCL);
                  if (!IsTestOk(Result, ts)) {
                    fprintf(stderr, "\nFAILED %s\n", File.c_str());
                    fflush(stderr);
                    testOk = false;
                  }
                  if (IF != FileCopy || !SaveTemps) {
                    TempFileRemove(IF);
                  }
                  IF = OF;
                }
              }
            }
          }
        }
      }
    }

    if (testOk) {
      vector<string> DiffCL;
      if (GetDiffCommandLine(DiffCL, File, FileCopy)) {
        if (Verbose) {
          cout << "Diff Command line" << std::endl;
          for (auto s : DiffCL)
            cout << s << " ";
          cout << std::endl;
        }

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

bail:
  lua_cleanup();

  return Ret;
}
