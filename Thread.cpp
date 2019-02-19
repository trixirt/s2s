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
#include <assert.h>
#include <windows.h>

#if _DEBUG
static void DebugIOFailure(HANDLE pipe) {
  DWORD lastError = GetLastError();
  // It is expected that an exiting child will close its pipe
  // So do not report these as errors
  if (lastError != ERROR_BROKEN_PIPE) {
    wchar_t *message = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&message, 0, NULL);
    if (message != nullptr) {
      fwprintf(stderr, L"%x %s", (unsigned)pipe, message);
      fflush(stderr);
      LocalFree(message);
    }
  }
}
#else
#define DebugIOFailure(a)
#endif

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
      DebugIOFailure(ioParameters->pipe);
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
    BOOL status;
    ioParameters->pipe = pipe;
    ioParameters->file = file;
    ioParameters->std = std;
    thread = CreateThread(NULL, 0, ReadOutputWorkFunction,
                          static_cast<LPVOID>(ioParameters), 0, &threadId);
    assert(thread != INVALID_HANDLE_VALUE);
    status = SetHandleInformation(thread, HANDLE_FLAG_INHERIT, 0);
    assert(status == TRUE);
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

  DWORD bytesToWrite, bytesWritten, flags;
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
      if (status == TRUE) {
        rtotal += bytesWritten;
      } else {
        DebugIOFailure(ioParameters->pipe);
        break;
      }
    }
  }
  //
  // May need to close the pipe to signal to the child
  // that all the input has been written.  But if the child
  // exited early the handle is already closed.  So check..
  status = GetHandleInformation(ioParameters->pipe, &flags);
  if (status == TRUE)
    CloseHandle(ioParameters->pipe);
  delete ioParameters;
  return ret;
}

HANDLE WriteInput(HANDLE pipe, FILE *file) {
  HANDLE thread = INVALID_HANDLE_VALUE;
  DWORD threadId = 0;
  struct IOParameters *ioParameters = new struct IOParameters;
  if (ioParameters != nullptr) {
    BOOL status;
    ioParameters->pipe = pipe;
    ioParameters->file = file;
    ioParameters->std = nullptr;
    thread = CreateThread(NULL, 0, WriteInputWorkFunction,
                          static_cast<LPVOID>(ioParameters), 0, &threadId);
    assert(thread != INVALID_HANDLE_VALUE);
    status = SetHandleInformation(thread, HANDLE_FLAG_INHERIT, 0);
    assert(status == TRUE);
  }
  return thread;
}
#endif
