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
#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <string>
#include <vector>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

using namespace std;

lua_State *lua_get();
void lua_init();
void lua_cleanup();
int lua_file(const char *);

bool lua_get_int(const char *func, int &O);
bool lua_get_int(const char *func, int &O, int &I);
bool lua_get_int(const char *func, int &O, int &I, string &IS);
bool lua_get_int(const char *func, int &O, vector<string> &IL, string &IS1,
                 string &IS2, string &IS3);
bool lua_get_list(const char *func, vector<string> &OL);
bool lua_get_list(const char *func, vector<string> &OL, string &IS);
bool lua_get_list(const char *func, vector<string> &OL, string &IS1,
                  string &IS2);
bool lua_get_list(const char *func, vector<string> &OL, string &IS1,
                  string &IS2, string &IS3);
bool lua_get_list(const char *func, vector<string> &OL, string &IS1,
                  string &IS2, string &IS3, string &IS4);
bool lua_get_list(const char *func, vector<string> &OL, string &IS1,
                  string &IS2, string &IS3, string &IS4, string &IS5);
bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL);
bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &IS1);
bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &IS1, string &IS2);
bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &IS1, string &IS2, string &IS3);
bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &IS1, string &IS2, string &IS3, string &IS4);
bool lua_get_string(const char *func, string &OS);
bool lua_get_string(const char *func, string &OL, string &IS);

#define LuaFilterDBEntry(O, ICL, IF, ID, X)                                    \
  lua_get_int("FilterDBEntry", O, ICL, IF, ID, X)

#define LuaGetDiffCommandLine(OCL, AF, BF)                                     \
  lua_get_list("GetDiffCommandLine", OCL, AF, BF)
#define LuaGetEditorCommandLine(OCL, ICL, IF, OF, X)                           \
  lua_get_list("GetEditorCommandLine", OCL, ICL, IF, OF, X)
#define LuaGetEditorExtension(OS) lua_get_string("GetEditorExtension", OS)
#define LuaGetS2SCommandLine(OCL, ICL, IF, OF, X)                              \
  lua_get_list("GetS2SCommandLine", OCL, ICL, IF, OF, X)
#define LuaGetS2SExtension(OS) lua_get_string("GetS2SExtension", OS)
#define LuaGetTestCommandLine(OCL, ICL, TC, TS, IF, OF)                        \
  lua_get_list("GetTestCommandLine", OCL, ICL, TC, TS, IF, OF)
#define LuaGetTestConfigurations(OL, X, E)                                     \
  lua_get_list("GetTestConfigurations", OL, X, E)
#define LuaGetTestExtension(OS, TS) lua_get_string("GetTestExtension", OS, TS)
#define LuaGetTestStages(OL, IS) lua_get_list("GetTestStages", OL, IS)

#define LuaIsDiffOk(O, I) lua_get_int("IsDiffOk", O, I)
#define LuaIsEditorOk(O, I) lua_get_int("IsEditorOk", O, I)
#define LuaIsS2SOk(O, I) lua_get_int("IsS2SOk", O, I)
#define LuaIsTestOk(O, I, TS) lua_get_int("IsTestOk", O, I, TS)
#define LuaIsOverWriteOk(O) lua_get_int("IsOverWriteOk", O)

#endif
