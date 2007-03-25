// BEGIN_COPYRIGHT
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




// lex.h:  Simple lexical analysis utilities

// Given a string containing C-style tokens, returns information about
// successive tokens.  Doesn't support all C tokens; just the few that
// GLEAN needs.


#ifndef __lex_h__
#define __lex_h__

#include <string>

#ifdef __WIN__
// ERROR is already defined in some windows header file
#undef ERROR
#endif

namespace GLEAN {

class Lex {
    protected:

	// State information:
	const char* input;
	const char* p;
	bool ignoringCase;

    public:

    	// Constructors:

	Lex(const char* s, bool ignoreCase = false);
		// Creates a lexer which will draw input from the given string.

	// Exceptions:

	struct Error { };			// Base class for errors.
	struct Lexical: public Error {		// Lexical error in string.
		const char* err;
		int position;
		Lexical(const char* e, int p) {
			err = e;
			position = p;
		}
	};
	struct InternalError: public Error {	// Shouldn't happen.
	};

	// Tokens:

	typedef enum {
		ERROR = 0,	// erroneous token; must be zero
		END,		// end of input

		AND,		// &
		AND_AND,	// &&
		ASSIGN,		// =
		BANG,		// !
		COMMA,		// ,
		DOT,		// .
		EQ,		// ==
		GE,		// >=
		GT,		// >
		LE,		// <=
		LPAREN,		// (
		LT,		// <
		MINUS,		// -
		NE,		// !=
		OR,		// |
		OR_OR,		// ||
		PERCENT,	// %
		PLUS,		// +
		RPAREN,		// )
		SLASH,		// /
		STAR,		// *

		ICONST,		// signed integer constant

		ID		// identifier
	} Token;

	// Utilities:

	void next();		// fetch next token

	Token token;		// current token
	std::string id;		// most recent identifier
	int iValue;		// most recent signed integer value

	inline int position() {	// current position in input string
		return p - input;
	}
}; // class Lex

} // namespace GLEAN

#endif // __lex_h__
