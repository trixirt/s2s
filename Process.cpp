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
#include <algorithm>
#include <errno.h>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace std;

static int _Process(vector<string> &A, string &StdIn, string &StdOut,
                    string &StdErr) {
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
    std::vector<const char *> Argv(A.size() + 1);
    std::transform(A.begin(), A.end(), Argv.begin(),
                   [](std::string &str) { return str.c_str(); });

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

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int select_status;
    int wait_status;
    size_t size = 4095;
    char buffer[4096];
    ssize_t count;
    char rbuffer[4096];
    ssize_t rcount, rtotal;

    bool done = false;
    rcount = rtotal = 0;

    FILE *childin = nullptr;
    if (StdIn.size())
      childin = fopen(StdIn.c_str(), "rt");

    FILE *childout = nullptr;
    if (StdOut.size())
      childout = fopen(StdOut.c_str(), "wt");
    if (childout == nullptr)
      childout = stdout;

    FILE *childerr = nullptr;
    if (StdErr.size())
      childerr = fopen(StdErr.c_str(), "wt");
    if (childerr == nullptr)
      childerr = stdout;

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

        if (rcount != rtotal) {
          if (FD_ISSET(stdin_pipe[1], &write_fds)) {
            count = write(stdin_pipe[1], &rbuffer[rtotal], rcount - rtotal);
            if (count > 1) {
              rtotal += count;
            } else if (count < 0) {
              // TODO : HANDLE
              perror("Write error");
              done = true;
            }
          }
        }
        if (FD_ISSET(stderr_pipe[0], &read_fds)) {
          count = read(stderr_pipe[0], buffer, size);
          buffer[count] = 0;
          fprintf(childerr, "%s", buffer);
          if (childerr != stdout)
            fprintf(stdout, "%s", buffer);
        }
        if (FD_ISSET(stdout_pipe[0], &read_fds)) {
          count = read(stdout_pipe[0], buffer, size);
          buffer[count] = 0;
          fprintf(childout, "%s", buffer);
          if (childout != stdout)
            fprintf(stdout, "%s", buffer);
        }
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

    if (childin != nullptr)
      fclose(childin);

    if (childout != stdout)
      fclose(childout);

    if (childerr != stdout)
      fclose(childerr);
  }

  return WEXITSTATUS(Status);
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
