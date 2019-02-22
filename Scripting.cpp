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
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

static lua_State *L = NULL;

lua_State *lua_get() { return L; }

void lua_init() {
  if (L == NULL) {
    L = luaL_newstate();
    luaL_openlibs(L);
  }
}
void lua_cleanup() {
  if (L)
    lua_close(L);
  L = NULL;
}

int lua_file(const char *file) {
  int r = -1;
  if (L) {
    if (luaL_loadfile(L, file) || lua_pcall(L, 0, 0, 0)) {
      fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
      lua_pop(L, 1);
    } else {
      r = 0;
    }
  }
  return r;
}

static bool getInt(int &O, int idx) {
  bool ret = false;
  if (lua_isnumber(L, idx)) {
    O = lua_tointeger(L, idx);
    ret = true;
  }
  return ret;
}

static bool getStrings(vector<string> &S, int idx) {
  bool ret = false;
  S.clear();
  if (lua_istable(L, idx)) {
    signed i = 0;
    while (++i) {
      lua_pushinteger(L, i);
      lua_gettable(L, idx - 1);
      if (!lua_isstring(L, -1))
        i = -1;
      else
        S.push_back(lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
  if (S.size())
    ret = true;
  return ret;
}

static bool putStrings(vector<string> &S) {
  bool ret = false;
  if (S.size()) {
    lua_newtable(L);
    signed i = 0;
    for (auto s : S) {
      ++i;
      lua_pushinteger(L, i);
      lua_pushstring(L, s.c_str());
      lua_settable(L, -3);
    }
    ret = true;
  }
  return ret;
}

static bool getString(string &S, int idx) {
  bool ret = false;
  if (lua_isstring(L, idx)) {
    S = lua_tostring(L, idx);
    ret = true;
  }
  return ret;
}

bool lua_get_int(const char *func, int &O) {
  bool ret = false;
  lua_getglobal(L, func);
  if (lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getInt(O, -1);
  }
  return ret;
}

bool lua_get_int(const char *func, int &O, int &I) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  lua_pushinteger(L, I);
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getInt(O, -1);
  }
  return ret;
}

bool lua_get_int(const char *func, int &O, int &I, string &S1) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  lua_pushinteger(L, I);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getInt(O, -1);
  }
  return ret;
}

bool lua_get_int(const char *func, int &O, vector<string> &IL, string &S1,
                 string &S2, string &S3) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getInt(O, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL) {
  bool ret = false;
  lua_getglobal(L, func);
  if (lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, string &S) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  lua_pushstring(L, S.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, string &S1,
                  string &S2) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, string &S1, string &S2,
                  string &S3) {
  bool ret = false;
  lua_getglobal(L, func);
  int i = 0;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, string &S1, string &S2,
                  string &S3, string &S4) {
  bool ret = false;
  lua_getglobal(L, func);
  int i = 0;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  lua_pushstring(L, S4.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, string &S1, string &S2,
                  string &S3, string &S4, string &S5) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  lua_pushstring(L, S4.c_str());
  i++;
  lua_pushstring(L, S5.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &S1) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &S1, string &S2) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &S1, string &S2, string &S3) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &S1, string &S2, string &S3, string &S4) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  lua_pushstring(L, S4.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_list(const char *func, vector<string> &OL, vector<string> &IL,
                  string &S1, string &S2, string &S3, string &S4, string &S5) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  putStrings(IL);
  i++;
  lua_pushstring(L, S1.c_str());
  i++;
  lua_pushstring(L, S2.c_str());
  i++;
  lua_pushstring(L, S3.c_str());
  i++;
  lua_pushstring(L, S4.c_str());
  i++;
  lua_pushstring(L, S5.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getStrings(OL, -1);
  }
  return ret;
}

bool lua_get_string(const char *func, string &OS) {
  bool ret = false;
  lua_getglobal(L, func);
  if (lua_pcall(L, 0, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getString(OS, -1);
  }
  return ret;
}

bool lua_get_string(const char *func, string &OS, string &S1) {
  bool ret = false;
  int i = 0;
  lua_getglobal(L, func);
  lua_pushstring(L, S1.c_str());
  i++;
  if (lua_pcall(L, i, 1, 0)) {
    fprintf(stderr, "Error: %s \n", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    ret = getString(OS, -1);
  }
  return ret;
}
