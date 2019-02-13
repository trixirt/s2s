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
#ifdef WIN32
#include <malloc.h> // for _alloca use in boost
#endif
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <string.h>

void TempFileName(std::string &Ext, std::string &OF) {
  boost::filesystem::path p = boost::filesystem::temp_directory_path() /
                              boost::filesystem::unique_path();
  
  OF = p.string();
  if (Ext.size())
    OF = OF + Ext;
}

void TempFileName(const char *Ext, std::string &OF) {
  std::string e(Ext);
  TempFileName(e, OF);
}

void TempFileRemove(std::string &F) {
  boost::filesystem::path p = F;
  boost::filesystem::remove(p);
}

void TempFileCopy(std::string &OF, std::string &IF, std::string &Ext) {
  OF.clear();
  TempFileName(Ext, OF);
  if (OF.size()) {
    boost::system::error_code EC;
    boost::filesystem::path IP = IF;
    boost::filesystem::path OP = OF;
    boost::filesystem::copy_file(IP, OP, EC);

    if (EC) {
      TempFileRemove(OF);
      OF.clear();
      std::cout << EC.message() << std::endl;
    }
  }
}

void TempFileRename(std::string &OF, std::string &IF) {
  boost::filesystem::path IP = IF;
  boost::filesystem::path OP = OF;
  boost::filesystem::rename(IP, OP);
}

void TempFileOverWrite(std::string &OF, std::string &IF) {
  boost::system::error_code EC;
  boost::filesystem::path IP = IF;
  boost::filesystem::path OP = OF;
  TempFileRemove(OF);
  boost::filesystem::copy_file(IP, OP, EC);
  TempFileRemove(IF);
}

void TempFilePipeName(std::string &OF) {
	std::string X = "\\\\.\\pipe\\";
	boost::filesystem::path p = boost::filesystem::unique_path();
	
  OF = X + p.string();
}
