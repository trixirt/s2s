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
#ifndef THREAD_H
#define THREAD_H

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
struct IOParameters {
  HANDLE pipe;
  FILE *file;
  FILE *std;
};
HANDLE ReadOutput(HANDLE pipe, FILE *file, FILE *std);
HANDLE WriteInput(HANDLE pipe, FILE *file);

#endif
#endif
