// BEGIN_COPYRIGHT
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// multisample changes: Copyright (c) 2008 VMware, Inc.  All rights reserved.
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




// dsfilt.h:  Utilities for selecting (filtering) drawing surface configs

// Given a string representing a Boolean expression involving
// attributes of drawing surface configurations, construct an internal
// representation of the expression which can be used to find matching
// configurations.  The string may also include sorting criteria that
// will be used to select the order in which matching configurations
// are returned.

// This class accepts a superset of the criteria supported by the
// visinfo package, originally released by SGI and used in the isfast
// library (among other things).  Apologies for inconsistent naming
// conventions, capitalization, redundancy, etc.; they're due to an
// incomplete translation of the old C code.  Here's the original
// copyright from visinfo, just in case the lawyers are interested:

/*
 * Copyright (c) 1994 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and   
 * its documentation for any purpose is hereby granted without fee,
 * provided that (i) the above copyright notices and this permission
 * notice appear in all copies of the software and related documentation,
 * and (ii) the name of Silicon Graphics may not be used in any
 * advertising or publicity relating to the software without the specific,
 * prior written permission of Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *      
 * IN NO EVENT SHALL SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */


#ifndef __dsfilt_h__
#define __dsfilt_h__

#include <string>
#include <vector>
#include <map>
#include "lex.h"

using namespace std;

namespace GLEAN {

class DrawingSurfaceConfig;	// forward reference

class DrawingSurfaceFilter {
    public:

    	// Constructors:

	DrawingSurfaceFilter(const string& s);
		// Creates a DrawingSurfaceFilter that implements the
		// filtering and sorting criteria in the given string.

	// Exceptions:

	struct Error { };			// Base class for errors.
	struct Syntax: public Error {		// Syntax error in string.
		const char* err;
		int position;
		Syntax(const char* e, int p) {
			err = e;
			position = p;
		}
	};
	struct InternalError: public Error {	// Shouldn't happen.
	};

	// Utilities:

	bool matches(DrawingSurfaceConfig& c);
		// Returns true if the given DrawingSurfaceConfig matches
		// the filter criteria.

	vector<DrawingSurfaceConfig*> filter(vector<DrawingSurfaceConfig*>& v);
		// Returns a vector of DrawingSurfaceConfig pointers that
		// match the filter criteria, sorted according to the sorting
		// criteria.

    protected:

	typedef enum {
		// These are items that may appear in the parsed
		// representations of the filter or sort keys.

		// First, some special cases:
		ERROR = 0,	// erroneous token; must appear first
		END,		// end of expression

		// Next, arithmetic and Boolean operators:

		ADD,		// C integer +
		AND,		// C integer &&
		DIV,		// C integer /
		EQ,		// C integer ==
		GE,		// C integer >=
		GT,		// C integer >
		LE,		// C integer <=
		LT,		// C integer <
		MOD,		// C integer %
		MUL,		// C integer *
		NE,		// C integer !=
		NEGATE,		// C integer unary -
		NOT,		// C integer unary !
		OR,		// C integer ||
		SUB,		// C integer -
		SEPARATOR,	// comma, separating exprs and sort keys
		LPAREN,		// (
		RPAREN,		// )

		// Sort keys:

		MAX,		// sort largest value first
		MIN,		// sort smallest value first

		// Finally, operands:

		CONSTANT,	// integer constants

		FIRST_VAR,	// marker; starts list of variables

		VAR_R,		// red channel size
		VAR_G,		// green channel size
		VAR_B,		// blue channel size
		VAR_A,		// alpha channel size
		VAR_RGB,	// min(r, g, b)
		VAR_RGBA,	// min(r, g, b, a)

		VAR_CI,		// color index channel size

		VAR_ACCUM_R,	// accum buf red channel size
		VAR_ACCUM_G,	// accum buf green channel size
		VAR_ACCUM_B,	// accum buf blue channel size
		VAR_ACCUM_A,	// accum buf alpha channel size
		VAR_ACCUM_RGB,	// min(accum r, accum g, accum b)
		VAR_ACCUM_RGBA,	// min(accum r, accum g, accum b, accum a)

		VAR_SAMPLES,    // number of samples per pixel

		VAR_AUX,	// number of aux color buffers

		VAR_DB,		// nonzero if double buffered
		VAR_SB,		// nonzero if single buffered

		VAR_ID,		// X11 Visual or Win32 PixelFormat ID
		VAR_FBCID,	// GLXFBConfig ID

		VAR_LEVEL,	// framebuffer level
		VAR_MAIN,	// nonzero for main buffers
		VAR_OVERLAY,	// nonzero for overlay buffers
		VAR_UNDERLAY,	// nonzero for underlay buffers

		VAR_MONO,	// nonzero for monoscopic buffers
		VAR_STEREO,	// nonzero for stereoscopic buffers

		VAR_MS,		// number of multisamples

		VAR_S,		// stencil buffer depth

		VAR_Z,		// depth (z) buffer depth

		VAR_FAST,	// nonzero if accelerated (or not marked
				// ``slow'' in GLX)

		VAR_CONFORMANT,	// nonzero if conforms to OpenGL spec

		VAR_TRANSPARENT,	// nonzero if some pixel value is
					// transparent
		VAR_TRANS_R,	// transparent value red component
		VAR_TRANS_G,	// transparent value green component
		VAR_TRANS_B,	// transparent value blue component
		VAR_TRANS_A,	// transparent value alpha component
		VAR_TRANS_CI,	// transparent value color index

		VAR_WINDOW,	// nonzero if can be used to create a window
		VAR_PBUFFER,	// nonzero if can be used to create a pbuffer
		VAR_PIXMAP,	// nonzero if can be used to create a pixmap
				// XXXWIN need VAR_BITMAP, at least;
				// possibly others

		VAR_GL_ONLY,	// nonzero if only OpenGL can render into
				// surfaces created with this config (i.e.,
				// the native window system *can't* render
				// into them).

		LAST_VAR	// marker; ends list of variables
	} Token;

	vector<int> condition;
	inline void Emit(Token op) {condition.push_back(static_cast<int>(op));}
	inline void Emit(int v) {condition.push_back(v);}
	vector<Token> sortKeys;
	inline void EmitKey(Token key) {sortKeys.push_back(key);}

	// Expression-parsing state and utilities:
	Lex lex;
	Token Symbol;
	int Value;
	static map<string,Token> varTable;
	static bool varTableInitialized;

	static int FetchVariable(const DrawingSurfaceConfig& c, Token v);
	static void InitVarTable();
	bool ParseArithExpr();
	bool ParseArithFactor();
	bool ParseArithPrimary();
	bool ParseArithTerm();
	bool ParseBoolFactor();
	bool ParseBoolTerm();
	bool ParseCriteria();
	bool ParseCriterion();
	bool ParseExpression();
	bool ParseSortKey();
	void GetSymbol();

	class ConfigSort {	// comparison function-object used for sorting
	    protected:
		vector<Token>& keys;
	    public:
		ConfigSort(vector<Token>& k): keys(k) { }
		bool operator() (const DrawingSurfaceConfig* c1,
			const DrawingSurfaceConfig* c2);
	};
	friend class ConfigSort;

}; // class DrawingSurfaceFilter

} // namespace GLEAN

#endif // __dsfilt_h__
