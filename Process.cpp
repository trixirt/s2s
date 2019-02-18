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
#include "TempFile.h"
#include "Thread.h"
#include <algorithm>
#include <errno.h>
#include <assert.h>
#include <iostream>
#include <string>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif
#include <vector>
using namespace std;

static int _Process(vector<string> &A, string &StdIn, string &StdOut,
                    string &StdErr) {
  int ret = -1;
  std::vector<const char *> Argv(A.size() + 1);
  std::transform(A.begin(), A.end(), Argv.begin(),
                 [](std::string &str) { return str.c_str(); });

  std::string Args;
  for (auto a : A) {
    Args += a;
    Args += " ";
  }

  FILE *childin = nullptr;
  if (StdIn.size()) {
    childin = fopen(StdIn.c_str(), "rt");
    assert(childin != nullptr);
  }

  FILE *childout = nullptr;
  if (StdOut.size()) {
    childout = fopen(StdOut.c_str(), "wt");
    assert(childout != nullptr);
  }
  if (childout == nullptr)
    childout = stdout;

  FILE *childerr = nullptr;
  if (StdErr.size()) {
    childerr = fopen(StdErr.c_str(), "wt");
    assert(childerr != nullptr);
  }
  if (childerr == nullptr)
    childerr = stdout;

  bool done = false;

#ifdef WIN32
  // From
  // https://docs.microsoft.com/en-us/windows/desktop/procthread/creating-a-child-process-with-redirected-input-and-output
  BOOL status;

  // Set the bInheritHandle flag so pipe handles are inherited.
  SECURITY_ATTRIBUTES securityAttributes;
  securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  securityAttributes.bInheritHandle = TRUE;
  securityAttributes.lpSecurityDescriptor = NULL;

  HANDLE stdin_pipe[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
  // create stdin pipe, 0 = read, 1 = write
  status = CreatePipe(&stdin_pipe[0], &stdin_pipe[1], &securityAttributes, 0);
  assert(status == TRUE);
  // Ensure the write handle to the pipe for STDIN is not inherited.
  status = SetHandleInformation(stdin_pipe[1], HANDLE_FLAG_INHERIT, 0);
  assert(status == TRUE);

  HANDLE stdout_pipe[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
  // create stdout pipe, 0 = read, 1 = write
  status = CreatePipe(&stdout_pipe[0], &stdout_pipe[1], &securityAttributes, 0);
  assert(status == TRUE);
  // Ensure the read handle to the pipe for STDOUT is not inherited.
  status = SetHandleInformation(stdout_pipe[0], HANDLE_FLAG_INHERIT, 0);
  assert(status == TRUE);

  HANDLE stderr_pipe[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
  // create stderr pipe, 0 = read, 1 = write
  status = CreatePipe(&stderr_pipe[0], &stderr_pipe[1], &securityAttributes, 0);
  assert(status == TRUE);
  // Ensure the read handle to the pipe for STDERR is not inherited.
  status = SetHandleInformation(stderr_pipe[0], HANDLE_FLAG_INHERIT, 0);
  assert(status == TRUE);

  DWORD ioHandles = 0;
  HANDLE ioWaitHandles[3] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
                             INVALID_HANDLE_VALUE};
  HANDLE ioThread;
  ioThread = ReadOutput(stderr_pipe[0], childerr, stderr);
  if (ioThread != INVALID_HANDLE_VALUE)
    ioWaitHandles[ioHandles++] = ioThread;
  ioThread = ReadOutput(stdout_pipe[0], childout, stdout);
  if (ioThread != INVALID_HANDLE_VALUE)
    ioWaitHandles[ioHandles++] = ioThread;
  if (childin != nullptr) {
    ioThread = WriteInput(stdin_pipe[1], childin);
    if (ioThread != INVALID_HANDLE_VALUE)
      ioWaitHandles[ioHandles++] = ioThread;
  }

  STARTUPINFOA startupInfo = {0};
  startupInfo.cb = sizeof(STARTUPINFO);
  startupInfo.hStdError = stderr_pipe[1];
  startupInfo.hStdOutput = stdout_pipe[1];
  startupInfo.hStdInput = stdin_pipe[0];
  startupInfo.dwFlags |= STARTF_USESTDHANDLES;

  PROCESS_INFORMATION processInformation;
  LPSTR commandLine = _strdup(Args.c_str());
  if (commandLine != nullptr) {
    status = CreateProcessA(NULL, commandLine,
                            NULL, // process security attributes
                            NULL, // primary thread security attributes
                            TRUE, // handles are inherited
                            0,    // creation flags
                            NULL, // use parent's environment
                            NULL, // use parent's current directory
                            &startupInfo, &processInformation);
    if (status == TRUE) {
      DWORD waitStatus, exitCode;

      if (ioHandles > 0)
        WaitForMultipleObjects(ioHandles, &ioWaitHandles[0], TRUE, INFINITY);

      waitStatus = WaitForSingleObject(processInformation.hProcess, INFINITY);
      status = GetExitCodeProcess(processInformation.hProcess, &exitCode);
      if (status == TRUE)
        ret = static_cast<int>(exitCode);
    }
    free(commandLine);
  }

  // Order pipe closing so child side is closed first
  HANDLE pipes[6] = {stdin_pipe[0],  stdin_pipe[1],  stdout_pipe[1],
                     stdout_pipe[0], stderr_pipe[1], stderr_pipe[0]};
  for (unsigned idx = 0; idx < 6; idx++) {
    DWORD flags = 0;
    status = GetHandleInformation(pipes[idx], &flags);
    if (status == TRUE)
      CloseHandle(pipes[idx]);
  }

#else
  int Status = -1;
  int stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
  pipe(stdin_pipe);
  pipe(stdout_pipe);
  pipe(stderr_pipe);

  pid_t pid;
  pid = fork();
  if (pid == -1) {
    exit(1);
  } else if (pid == 0) {
    /* child */
    int exec_status;

    dup2(stdin_pipe[0], STDIN_FILENO);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    dup2(stderr_pipe[1], STDERR_FILENO);

    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);

    for (auto a : A)
      std::cout << a << " ";
    std::cout << std::endl;

    exec_status = execvp(Argv[0], const_cast<char **>(Argv.data()));
    if (exec_status < 0) {
      fprintf(stderr, "Problem with exec of %s\n", Argv[0]);
      perror("");
    }
    _exit(1); /* no flush                             */
  } else {
    fd_set read_fds, write_fds;
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = 10;

    int select_status;
    int wait_status;

    size_t size = 4095;
    char buffer[4096];
    ssize_t count;
    char rbuffer[4096];
    ssize_t rcount, rtotal;
    rcount = rtotal = 0;

    unsigned no_work_done = 0;

    do {

      if (childin != nullptr) {
        if (!feof(childin)) {
          if (rcount == rtotal) {
            rcount = fread(rbuffer, 1, size, childin);
            rtotal = 0;
          }
        }
      }

      FD_ZERO(&read_fds);
      FD_SET(stdout_pipe[0], &read_fds);
      FD_SET(stderr_pipe[0], &read_fds);
      FD_ZERO(&write_fds);
      if (rcount != rtotal)
        FD_SET(stdin_pipe[1], &write_fds);
      errno = 0;
      select_status = select(FD_SETSIZE, &read_fds, &write_fds, NULL, &tv);
      if (select_status > 0) {
        no_work_done = 0;
        if (rcount != rtotal) {
          if (FD_ISSET(stdin_pipe[1], &write_fds)) {
            count = write(stdin_pipe[1], &rbuffer[rtotal], rcount - rtotal);
            if (count > 1) {
              rtotal += count;
            } else if (count < 0) {
              // TODO : HANDLE
              perror("Write error");
              fflush(stderr);
              done = true;
            }
          }
        }
        if (FD_ISSET(stderr_pipe[0], &read_fds)) {
          count = read(stderr_pipe[0], buffer, size);
          buffer[count] = 0;
          fprintf(childerr, "%s", buffer);
          fflush(childerr);
          if (childerr != stdout) {
            fprintf(stdout, "%s", buffer);
            fflush(stdout);
          }
        }
        if (FD_ISSET(stdout_pipe[0], &read_fds)) {
          count = read(stdout_pipe[0], buffer, size);
          buffer[count] = 0;
          fprintf(childout, "%s", buffer);
          fflush(childout);
          if (childout != stdout) {
            fprintf(stdout, "%s", buffer);
            fflush(stdout);
          }
        }
      } else {
        no_work_done++;
        usleep(10 * no_work_done);
        // cap at 2 sec
        if (no_work_done > 200)
          no_work_done = 0;
      }

      wait_status = waitpid(pid, &Status, WNOHANG);
      if (wait_status > 0) {
        done = true;
        continue;
      }

      if (childin != nullptr) {
        if ((rcount == rtotal) && (feof(childin))) {
          fclose(childin);
          childin = nullptr;
          close(stdin_pipe[1]);
        }
      }

      if (select_status == 0)
        if (done)
          break;
        else
          continue;

      if ((select_status < 1 && errno != EINTR))
        break;

    } while (1);

    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);
  }

  ret = WEXITSTATUS(Status);

#endif

  if (childin != nullptr)
    fclose(childin);

  if (childout != stdout)
    fclose(childout);

  if (childerr != stdout)
    fclose(childerr);

  return ret;
}

int Process(vector<string> &A, string &StdIn, string &StdOut, string &StdErr) {
  TempFileName(".stdout", StdOut);
  TempFileName(".stderr", StdErr);
  return _Process(A, StdIn, StdOut, StdErr);
}

int Process(vector<string> &A) {
  string dummy;
  return _Process(A, dummy, dummy, dummy);
}
