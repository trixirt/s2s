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
#ifndef TEMPFILE_H
#define TEMPFILE_H

#include <string>

void TempFileName(std::string &Ext, std::string &OF);
void TempFileName(const char *Ext, std::string &OF);
void TempFileRemove(std::string &F);
void TempFileCopy(std::string &OF, std::string &IF, std::string &Ext);
void TempFileOverWrite(std::string &OF, std::string &IF);
void TempFileRename(std::string &OF, std::string &IF);

#endif
