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
#include "Thread.h"
#ifdef WIN32
#include <windows.h>

static DWORD WINAPI ReadOutputWorkFunction(LPVOID param) {
  struct IOParameters *ioParameters = static_cast<struct IOParameters *>(param);
  DWORD ret = 0;

  DWORD bufferSize = 4095;
  char buffer[4096];
  DWORD bytesRead;
  BOOL status;
  while (1) {
    bytesRead = 0;
    status =
        ReadFile(ioParameters->pipe, &buffer[0], bufferSize, &bytesRead, NULL);
    if (status == TRUE) {
      if (bytesRead > 0) {
        buffer[bytesRead] = 0;
        fprintf(ioParameters->file, "%s", buffer);
        fflush(ioParameters->file);
        if (ioParameters->file != ioParameters->std) {
          fprintf(ioParameters->std, "%s", buffer);
          fflush(ioParameters->std);
        }
      }
    } else {
      break;
    }
  }
  delete ioParameters;
  return ret;
}

HANDLE ReadOutput(HANDLE pipe, FILE *file, FILE *std) {
  HANDLE thread = INVALID_HANDLE_VALUE;
  DWORD threadId = 0;
  struct IOParameters *ioParameters = new struct IOParameters;
  if (ioParameters != nullptr) {
    ioParameters->pipe = pipe;
    ioParameters->file = file;
    ioParameters->std = std;
    thread = CreateThread(NULL, 0, ReadOutputWorkFunction,
                          static_cast<LPVOID>(ioParameters), 0, &threadId);
  }
  return thread;
}

static DWORD WINAPI WriteInputWorkFunction(LPVOID param) {
  struct IOParameters *ioParameters = static_cast<struct IOParameters *>(param);
  DWORD ret = 0;
  char buffer[4096];
  size_t size, rcount, rtotal;
  rcount = rtotal = 0;
  size = 4095;

  DWORD bytesToWrite, bytesWritten;
  BOOL status;
  while (1) {
    if (!feof(ioParameters->file)) {
      if (rcount == rtotal) {
        rcount = fread(&buffer[0], 1, size, ioParameters->file);
        rtotal = 0;
      }
    } else {
      if (rcount == rtotal)
        break;
    }
    bytesToWrite = rcount - rtotal;
    if (bytesToWrite > 0) {
      bytesWritten = 0;
      status = WriteFile(ioParameters->pipe, &buffer[rtotal], bytesToWrite,
                         &bytesWritten, NULL);
      if (status == TRUE)
        rtotal += bytesWritten;
      else
        break;
    }
  }
  CloseHandle(ioParameters->pipe);
  delete ioParameters;
  return ret;
}

HANDLE WriteInput(HANDLE pipe, FILE *file) {
  HANDLE thread = INVALID_HANDLE_VALUE;
  DWORD threadId = 0;
  struct IOParameters *ioParameters = new struct IOParameters;
  if (ioParameters != nullptr) {
    ioParameters->pipe = pipe;
    ioParameters->file = file;
    ioParameters->std = nullptr;
    thread = CreateThread(NULL, 0, WriteInputWorkFunction,
                          static_cast<LPVOID>(ioParameters), 0, &threadId);
  }
  return thread;
}
#endif