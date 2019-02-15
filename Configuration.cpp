#include "Scripting.h"
#include <iostream>
using namespace std;

bool FilterDBEntry(std::vector<std::string> &ICL, std::string &IF,
                   std::string &ID, std::string &Exe) {
  bool ret = true;
  int O = -1;
  LuaFilterDBEntry(O, ICL, IF, ID, Exe);
  if (O != 1)
    ret = false;
  return ret;
}

bool GetTestConfigurations(vector<string> &TC, string &Exe, string &Ext) {
  bool ret = LuaGetTestConfigurations(TC, Exe, Ext);
  return ret;
}
bool GetTestStages(vector<string> &TS, string &TC) {
  bool ret = LuaGetTestStages(TS, TC);
  return ret;
}

bool GetTestCommandLine(vector<string> &OCL, vector<string> &ICL, string &TC,
                        string &TS, string &IF, string &OF) {
  bool ret = LuaGetTestCommandLine(OCL, ICL, TC, TS, IF, OF);
  return ret;
}

bool GetTestExtension(string &E, string &TS) {
  bool ret = LuaGetTestExtension(E, TS);
  return ret;
}

bool IsTestOk(int &I, string &TS) {
  bool ret = false;
  int O = -1;
  LuaIsTestOk(O, I, TS);
  if (O == 1)
    ret = true;
  return ret;
}

bool GetS2SCommandLine(vector<string> &OCL, vector<string> &ICL, string &IF,
                       string &OF, string &Exe) {
  bool ret = LuaGetS2SCommandLine(OCL, ICL, IF, OF, Exe);
  return ret;
}

bool GetS2SExtension(string &E) {
  bool ret = LuaGetS2SExtension(E);
  return ret;
}

bool IsS2SOk(int &I) {
  bool ret = false;
  int O = -1;
  LuaIsS2SOk(O, I);
  if (O == 1)
    ret = true;
  return ret;
}

bool GetEditorCommandLine(vector<string> &OCL, vector<string> &ICL, string &IF,
                          string &OF, string &Exe) {
  bool ret = LuaGetEditorCommandLine(OCL, ICL, IF, OF, Exe);
  return ret;
}

bool GetEditorExtension(string &E) {
  bool ret = LuaGetEditorExtension(E);
  return ret;
}

bool IsEditorOk(int &I) {
  bool ret = false;
  int O = -1;
  LuaIsEditorOk(O, I);
  if (O == 1)
    ret = true;
  return ret;
}

bool IsOverWriteOk() {
  bool ret = false;
  int O = -1;
  LuaIsOverWriteOk(O);
  if (O == 1)
    ret = true;
  return ret;
}

bool GetDiffCommandLine(vector<string> &OCL, string &AF, string &BF) {
  bool ret = LuaGetDiffCommandLine(OCL, AF, BF);
  return ret;
}

bool IsDiffOk(int &I) {
  bool ret = false;
  int O = -1;
  LuaIsDiffOk(O, I);
  if (O == 1)
    ret = true;
  return ret;
}
