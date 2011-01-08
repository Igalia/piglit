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




// lex.cpp:  Implementation of simple lexical analyzer

#include <ctype.h>
#include <stdlib.h>
#include "lex.h"

namespace GLEAN {


///////////////////////////////////////////////////////////////////////////////
// Constructor:
///////////////////////////////////////////////////////////////////////////////
Lex::Lex(const char* s, bool ignoreCase/* = false */) {
	input = s;
	p = input;
	ignoringCase = ignoreCase;
	token = ERROR;
	iValue = 0;
} // Lex::Lex

///////////////////////////////////////////////////////////////////////////////
// next - Fetch next token from the input string
///////////////////////////////////////////////////////////////////////////////
void
Lex::next() {
	while (isspace(*p))
		++p;

	if (isalpha(*p) || *p == '_') {
		id = "";
		if (ignoringCase)
			while (isalnum(*p) || *p == '_')
				id += tolower(*p++);
		else
			while (isalnum(*p) || *p == '_')
				id += *p++;
		token = ID;
		return;
	}

	if (isdigit(*p)) {
		iValue = strtol(p, const_cast<char**>(&p), 0);
		token = ICONST;
		return;
	}

	char nextC = 0;
	char c = *p++;
	if (c)
		nextC = *p;
	switch (c) {
	case '|':
		if (nextC == '|') {
			++p;
			token = OR_OR;
		}
		else
			token = OR;
		break;
	case '&':
		if (nextC == '&') {
			++p;
			token = AND_AND;
		}
		else
			token = AND;
		break;
	case '<':
		if (nextC == '=') {
			++p;
			token = LE;
		}
		else
			token = LT;
		break;
	case '>':
		if (nextC == '=') {
			++p;
			token = GE;
		}
		else
			token = GT;
		break;
	case '=':
		if (nextC == '=') {
			++p;
			token = EQ;
		}
		else
			token = ASSIGN;
		break;
	case '!':
		if (nextC == '=') {
			++p;
			token = NE;
		}
		else
			token = BANG;
		break;
	case '+':
		token = PLUS;
		break;
	case '-':
		token = MINUS;
		break;
	case '*':
		token = STAR;
		break;
	case '/':
		token = SLASH;
		break;
	case '%':
		token = PERCENT;
		break;
	case ',':
		token = COMMA;
		break;
	case '(':
		token = LPAREN;
		break;
	case ')':
		token = RPAREN;
		break;
	case '.':
		token = DOT;
		break;
	case '\0':
		token = END;
		--p;	// push back '\0' so that token will always be END
		break;
	default:
		throw Lexical("unrecognized symbol", p - input);
	}

	return;
} // Lex::next

} // namespace GLEAN
