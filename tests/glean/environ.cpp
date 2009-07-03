// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT




// environ.cpp:  implementation of test environment class

#include "environ.h"

#if defined(__UNIX__)
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <cstdio>
#elif defined(__MS__)

#include <sys/stat.h>

#endif

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
Environment::Environment(Options& opt):
    options(opt),
    log(cout),
    winSys(opt)
{
#   if defined(__UNIX__)

	// If running tests, first create the results directory.
	// Refuse to overwrite one that already exists.
	if (opt.mode == Options::run) {
		if (opt.overwrite) {
			// remove existing db dir
			// XXX using system() probably isn't ideal
			char cmd[1000];
			snprintf(cmd, 999, "rm -rf %s", opt.db1Name.c_str());
			system(cmd);
		}
		if (mkdir(opt.db1Name.c_str(), 0755)) {
			if (errno == EEXIST)
				throw DBExists();
			else
				throw DBCantOpen(opt.db1Name);
		}
	// If comparing previous runs, make a token attempt to verify
	// that the two databases exist.
	} else {
		struct stat s;
		if (stat(opt.db1Name.c_str(), &s) || !S_ISDIR(s.st_mode))
			throw DBCantOpen(opt.db1Name);
		if (stat(opt.db2Name.c_str(), &s) || !S_ISDIR(s.st_mode))
			throw DBCantOpen(opt.db2Name);
	}

#   elif defined(__MS__)
	// If running tests, first create the results directory.
	// Refuse to overwrite one that already exists.
	if (opt.mode == Options::run) {
		if (opt.overwrite) {
			char cmd[1000];
#if defined(_MSC_VER)
			_snprintf(cmd, 999, "rd /s /q %s", opt.db1Name.c_str());
#else
			snprintf(cmd, 999, "rd /s /q %s", opt.db1Name.c_str());
#endif
			system(cmd);
		}
		if (!CreateDirectory(opt.db1Name.c_str(),0)) {
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				throw DBExists();
			else
				throw DBCantOpen(opt.db1Name);
		}
	// If comparing previous runs, make a token attempt to verify
	// that the two databases exist.
	} else {
		struct _stat s; 

		if (_stat(opt.db1Name.c_str(), &s) || !(s.st_mode & _S_IFDIR))
			throw DBCantOpen(opt.db1Name);
		if (_stat(opt.db2Name.c_str(), &s) || !(s.st_mode & _S_IFDIR))
			throw DBCantOpen(opt.db2Name);
	}

#   endif
} // Environment::Environment()

///////////////////////////////////////////////////////////////////////////////
// Results-file access utilities
///////////////////////////////////////////////////////////////////////////////
string
Environment::resultFileName(string& dbName, string& testName) {
#   if defined(__UNIX__)
	string dirName(dbName + '/' + testName);
	if (mkdir(dirName.c_str(), 0755)) {
		if (errno != EEXIST)
			throw DBCantOpen(dirName);
	}
	string fileName(dirName + "/results");
#   elif defined(__MS__)
	string dirName(dbName + '/' + testName);
	if (!CreateDirectory(dirName.c_str(),0)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			throw DBCantOpen(dirName);
	}
	string fileName(dirName + "/results");
#   endif
	return fileName;
} // Environment::resultFileName

string
Environment::imageFileName(string& dbName, string& testName, int n) {
	char sn[4];
	sn[3] = 0;
	sn[2] = static_cast<char>('0' + n % 10);
	sn[1] = static_cast<char>('0' + (n / 10) % 10);
	sn[0] = static_cast<char>('0' + (n / 100) % 10);
#   if defined(__UNIX__)
	string fileName(dbName + '/' + testName + "/i" + sn + ".tif");
#   elif defined(__MS__)
	string fileName(dbName + '/' + testName + "/i" + sn + ".tif");
#   endif
	return fileName;
} // Environment::imageFileName

void
Environment::quiesce() {
	winSys.quiesce();
#   if defined(__UNIX__)
	sync();
#   elif defined(__MS__)
#   endif
} // Environment::quiesce

} // namespace GLEAN
