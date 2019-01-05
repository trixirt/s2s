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
#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>

int Process(std::vector<std::string> &A);
int Process(std::vector<std::string> &A, std::string &StdinFile,
            std::string &StdoutFile, std::string &StderrFile);

#endif
