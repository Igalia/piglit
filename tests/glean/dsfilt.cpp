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




// dsfilt.cpp:  Implementation of drawing surface configuration filtering

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <algorithm>
#include "dsfilt.h"
#include "dsconfig.h"

namespace GLEAN {


///////////////////////////////////////////////////////////////////////////////
// Constructor:
///////////////////////////////////////////////////////////////////////////////
DrawingSurfaceFilter::DrawingSurfaceFilter(const string& s):
    lex(s.c_str(), true) {

	if (!varTableInitialized)
		InitVarTable();

	try {
		GetSymbol();
		if (!ParseCriteria())
			throw Syntax("no criteria found", lex.position());
	}
	catch (Lex::Lexical e) {
		throw Syntax(e.err, e.position);
	}

	// Make the final sort in order of increasing ID number:
	EmitKey(MIN);
	EmitKey(VAR_ID);
#	if defined(GLX_VERSION_1_3)
		EmitKey(MIN);
		EmitKey(VAR_FBCID);
#	endif
} // DrawingSurfaceFilter::DrawingSurfaceFilter

///////////////////////////////////////////////////////////////////////////////
// matches - determine if a drawing surface config matches the specified
//	criteria
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::matches(DrawingSurfaceConfig& c) {
	// Process the RPN expression in ``condition'', using the supplied
	// drawing surface configuration to determine values of variables.

	vector<int> stack;

	for (vector<int>::const_iterator p = condition.begin();
	    p < condition.end(); ++p) {
	    	int right;

		switch (*p) {
		case ADD:
			right = stack.back(); stack.pop_back();
			stack.back() += right;
			break;
		case AND:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() && right;
			break;
		case DIV:
			right = stack.back(); stack.pop_back();
			stack.back() = (right == 0)? 0: stack.back() / right;
			break;
		case EQ:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() == right;
			break;
		case GE:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() >= right;
			break;
		case GT:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() > right;
			break;
		case LE:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() <= right;
			break;
		case LT:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() < right;
			break;
		case MOD:
			right = stack.back(); stack.pop_back();
			stack.back() = (right == 0)? 0: stack.back() % right;
			break;
		case MUL:
			right = stack.back(); stack.pop_back();
			stack.back() *= right;
			break;
		case NE:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() != right;
			break;
		case NEGATE:
			stack.back() = -stack.back();
			break;
		case NOT:
			stack.back() = !stack.back();
			break;
		case OR:
			right = stack.back(); stack.pop_back();
			stack.back() = stack.back() || right;
			break;
		case SUB:
			right = stack.back(); stack.pop_back();
			stack.back() -= right;
			break;
		case CONSTANT:
			stack.push_back(*++p);
			break;
		default:
			// Must be a variable.
			stack.push_back(FetchVariable(c,
				static_cast<Token>(*p)));
			break;
		}
	}

	return stack.back();
} // DrawingSurfaceFilter::matches

///////////////////////////////////////////////////////////////////////////////
// filter - find and sort all matching drawing surface configurations
///////////////////////////////////////////////////////////////////////////////
vector<DrawingSurfaceConfig*>
DrawingSurfaceFilter::filter(vector<DrawingSurfaceConfig*>& v) {
	vector<DrawingSurfaceConfig*> result;
	for (vector<DrawingSurfaceConfig*>::const_iterator p = v.begin();
	    p < v.end(); ++p) {
	 	if (matches(**p))
			result.push_back(*p);
	}

	sort(result.begin(), result.end(), ConfigSort(sortKeys));
	return result;
} // DrawingSurfaceFilter::filter

///////////////////////////////////////////////////////////////////////////////
// ConfigSort operator() - sort comparison for final ordering of configurations
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ConfigSort::operator()
    (const DrawingSurfaceConfig* c1, const DrawingSurfaceConfig* c2) {
	for (vector<Token>::const_iterator p=keys.begin(); p<keys.end(); ++p) {
		Token dir = *p++;
		int d = FetchVariable(*c1, *p) - FetchVariable(*c2, *p);
		if (dir == MAX)		// sort largest first?
			d = -d;
		if (d < 0)
			return true;	// c1 goes before c2
		if (d > 0)
			return false;	// c1 goes after c2
	}
	return false;			// order doesn't matter
}

///////////////////////////////////////////////////////////////////////////////
// InitVarTable - initialize the table mapping variable names to token values
///////////////////////////////////////////////////////////////////////////////

map<string,DrawingSurfaceFilter::Token> DrawingSurfaceFilter::varTable;
bool DrawingSurfaceFilter::varTableInitialized;

void
DrawingSurfaceFilter::InitVarTable() {
	varTable["r"] =			VAR_R;
	varTable["g"] =			VAR_G;
	varTable["b"] =			VAR_B;
	varTable["a"] =			VAR_A;
	varTable["rgb"] =		VAR_RGB;
	varTable["rgba"] =		VAR_RGBA;
	varTable["ci"] =		VAR_CI;
	varTable["accumr"] =		VAR_ACCUM_R;
	varTable["accumg"] =		VAR_ACCUM_G;
	varTable["accumb"] =		VAR_ACCUM_B;
	varTable["accuma"] =		VAR_ACCUM_A;
	varTable["accumrgb"] =		VAR_ACCUM_RGB;
	varTable["accumrgba"] =		VAR_ACCUM_RGBA;
	varTable["aux"] =		VAR_AUX;
	varTable["db"] =		VAR_DB;
	varTable["sb"] =		VAR_SB;
	varTable["id"] =		VAR_ID;
	varTable["fbcid"] =		VAR_FBCID;
	varTable["level"] =		VAR_LEVEL;
	varTable["main"] =		VAR_MAIN;
	varTable["overlay"] =		VAR_OVERLAY;
	varTable["underlay"] =		VAR_UNDERLAY;
	varTable["mono"] =		VAR_MONO;
	varTable["stereo"] =		VAR_STEREO;
	varTable["ms"] =		VAR_MS;
	varTable["s"] =			VAR_S;
	varTable["z"] =			VAR_Z;
	varTable["fast"] =		VAR_FAST;
	varTable["conformant"] =	VAR_CONFORMANT;
	varTable["transparent"] =	VAR_TRANSPARENT;
	varTable["transr"] =		VAR_TRANS_R;
	varTable["transg"] =		VAR_TRANS_G;
	varTable["transb"] =		VAR_TRANS_B;
	varTable["transa"] =		VAR_TRANS_A;
	varTable["transci"] =		VAR_TRANS_CI;
	varTable["window"] =		VAR_WINDOW;
	varTable["pbuffer"] =		VAR_PBUFFER;
	varTable["pixmap"] =		VAR_PIXMAP;
	varTable["glonly"] =		VAR_GL_ONLY;
	varTable["max"] =		MAX;
	varTable["min"] =		MIN;

	varTableInitialized = true;
} // DrawingSurfaceFilter::InitVarTable

///////////////////////////////////////////////////////////////////////////////
// FetchVariable - fetch the value of a variable from a
//	DrawingSurfaceConfig
///////////////////////////////////////////////////////////////////////////////

int
DrawingSurfaceFilter::FetchVariable(const DrawingSurfaceConfig& c, Token v) {
	switch (v) {
	case VAR_R:
		return c.r;
	case VAR_G:
		return c.g;
	case VAR_B:
		return c.b;
	case VAR_A:
		return c.a;
	case VAR_RGB:
		return min(c.r, min(c.g, c.b));
	case VAR_RGBA:
		return min(c.r, min(c.g, min(c.b, c.a)));

	case VAR_CI:
		return c.canCI? c.bufSize: 0;

	case VAR_ACCUM_R:
		return c.accR;
	case VAR_ACCUM_G:
		return c.accG;
	case VAR_ACCUM_B:
		return c.accB;
	case VAR_ACCUM_A:
		return c.accA;
	case VAR_ACCUM_RGB:
		return min(c.accR, min(c.accG, c.accB));
	case VAR_ACCUM_RGBA:
		return min(c.accR, min(c.accG, min(c.accB, c.accA)));

	case VAR_AUX:
		return c.aux;

	case VAR_DB:
		return c.db;
	case VAR_SB:
		return !c.db;

	case VAR_ID:
#		if defined(__X11__)
			return c.visID;
#		elif defined(__WIN__)
			return c.pfdID;
#		endif
	case VAR_FBCID:
#		if defined(GLX_VERSION_1_3)
			return c.fbcID;
#		else
			return 0;
#		endif

	case VAR_LEVEL:
		return c.level;
	case VAR_MAIN:
		return c.level == 0;
	case VAR_OVERLAY:
		return c.level > 0;
	case VAR_UNDERLAY:
		return c.level < 0;

	case VAR_MONO:
		return !c.stereo;
		break;
	case VAR_STEREO:
		return c.stereo;

	case VAR_MS:
		// XXX Can't support this at the moment; have no way to
		// compile or test.
		return 0;

	case VAR_S:
		return c.s;

	case VAR_Z:
		return c.z;

	case VAR_FAST:
		return c.fast;
	case VAR_CONFORMANT:
		return c.conformant;

	case VAR_TRANSPARENT:
		return c.transparent;
	case VAR_TRANS_R:
		return c.transR;
	case VAR_TRANS_G:
		return c.transG;
	case VAR_TRANS_B:
		return c.transB;
	case VAR_TRANS_A:
		return c.transA;
	case VAR_TRANS_CI:
		return c.transI;

	case VAR_WINDOW:
		return c.canWindow;
	case VAR_PBUFFER:
#		if defined(GLX_VERSION_1_3)
			return c.canPBuffer;
#		else
			return 0;
#		endif
	case VAR_PIXMAP:
#		if defined(__X11__)
			return c.canPixmap;
#		else
			return 0;
#		endif

	case VAR_GL_ONLY:
		return !c.canWinSysRender;

	default:
		throw InternalError();
	}
}

///////////////////////////////////////////////////////////////////////////////
// GetSymbol - Fetch next symbol from the input string
///////////////////////////////////////////////////////////////////////////////
void
DrawingSurfaceFilter::GetSymbol() {
	lex.next();
	switch(lex.token) {
	case Lex::ID:
		Symbol = varTable[lex.id];
			// Note:  Because ERROR has value zero in the
			// Token enumeration, if the user provides a
			// variable that is not in varTable, then Symbol
			// will be set to ERROR.
		if (Symbol == ERROR)
			throw Syntax("unrecognized variable", lex.position());
		break;
	case Lex::ICONST:
		Value = lex.iValue;
		Symbol = CONSTANT;
		break;
	case Lex::OR_OR:
		Symbol = OR;
		break;
	case Lex::AND_AND:
		Symbol = AND;
		break;
	case Lex::LE:
		Symbol = LE;
		break;
	case Lex::LT:
		Symbol = LT;
		break;
	case Lex::GE:
		Symbol = GE;
		break;
	case Lex::GT:
		Symbol = GT;
		break;
	case Lex::EQ:
		Symbol = EQ;
		break;
	case Lex::NE:
		Symbol = NE;
		break;
	case Lex::BANG:
		Symbol = NOT;
		break;
	case Lex::PLUS:
		Symbol = ADD;
		break;
	case Lex::MINUS:
		Symbol = SUB;
		break;
	case Lex::STAR:
		Symbol = MUL;
		break;
	case Lex::SLASH:
		Symbol = DIV;
		break;
	case Lex::PERCENT:
		Symbol = MOD;
		break;
	case Lex::COMMA:
		Symbol = SEPARATOR;
		break;
	case Lex::LPAREN:
		Symbol = LPAREN;
		break;
	case Lex::RPAREN:
		Symbol = RPAREN;
		break;
	case Lex::END:
		Symbol = END;
		break;
	default:
		throw Syntax("unrecognized symbol", lex.position());
	}

	return;
} // DrawingSurfaceFilter::GetSymbol

///////////////////////////////////////////////////////////////////////////////
// ParseArithExpr
//	Syntax:	arithExpr -> arithTerm {('+'|'-') arithTerm}
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseArithExpr() {
	if (!ParseArithTerm())
		return false;

	for (;;) {
		if (Symbol == ADD || Symbol == SUB) {
			Token op = Symbol;
			GetSymbol();
			if (!ParseArithTerm())
				throw Syntax("missing operand of + or -",
					lex.position());
			Emit(op);
		} else
			return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ParseArithFactor
//	Syntax:	arithFactor -> ['+'|'-'|'!'] arithPrimary
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseArithFactor() {
	if (Symbol == ADD || Symbol == SUB || Symbol == NOT) {
		Token op = Symbol;
		GetSymbol();
		if (!ParseArithPrimary())
			throw Syntax("missing operand of unary +, -, or !",
				lex.position());
		if (op == SUB)
			Emit(NEGATE);
		else if (op == NOT)
			Emit(NOT);
		return true;
	}

	return ParseArithPrimary();
}

///////////////////////////////////////////////////////////////////////////////
// ParseArithPrimary
//	Syntax:	arithPrimary -> variable | constant | '(' expression ')'
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseArithPrimary() {
	if (FIRST_VAR < Symbol && Symbol < LAST_VAR) {
		Emit(Symbol);
		GetSymbol();
		return true;
	}

	if (Symbol == CONSTANT) {
		Emit(CONSTANT);
		Emit(Value);
		GetSymbol();
		return true;
	}

	if (Symbol == LPAREN) {
		GetSymbol();
		if (!ParseExpression())
			throw Syntax("missing expression after (",
				lex.position());
		if (Symbol == RPAREN) {
			GetSymbol();
			return true;
		} else
			throw Syntax("missing )", lex.position());
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// ParseArithTerm
//	Syntax:	arithTerm -> arithFactor {('*'|'/'|'%') arithFactor}
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseArithTerm() {
	if (!ParseArithFactor())
		return false;

	for (;;) {
		if (Symbol == MUL
		 || Symbol == DIV
		 || Symbol == MOD) {
			Token op = Symbol;
			GetSymbol();
			if (!ParseArithFactor())
				throw Syntax("missing operand of *, /, or %",
					lex.position());
			Emit(op);
		} else
			return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ParseBoolFactor
//   Syntax:  boolFactor -> arithExpr [('<'|'>'|'<='|'>='|'=='|'!=') arithExpr]
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseBoolFactor() {
	if (!ParseArithExpr())
		return false;

	if (Symbol == LT
	 || Symbol == GT
	 || Symbol == LE
	 || Symbol == GE
	 || Symbol == EQ
	 || Symbol == NE) {
		Token op = Symbol;
		GetSymbol();
		if (!ParseArithExpr())
			throw Syntax("missing operand of comparison",
				lex.position());
		Emit(op);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// ParseBoolTerm
//	Syntax:	boolTerm -> boolFactor {'&&' boolFactor}
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseBoolTerm() {
	if (!ParseBoolFactor())
		return false;

	for (;;) {
		if (Symbol == AND) {
			GetSymbol();
			if (!ParseBoolFactor())
				throw Syntax("missing operand of &&",
					lex.position());
			Emit(AND);
		} else
			return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ParseCriteria
//	Syntax:  criteria -> criterion {',' criterion}
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseCriteria() {
	/* Process all the user-specified conditions and sort keys: */
	if (!ParseCriterion())
		return false;

	for (;;) {
		if (Symbol == SEPARATOR) {
			GetSymbol();
			if (!ParseCriterion())
				throw Syntax("missing criterion after comma",
					lex.position());
			Emit(AND);
		} else if (Symbol == END)
			return true;
		else
			throw Syntax("expected comma or end of criteria",
				lex.position());
	}
}

///////////////////////////////////////////////////////////////////////////////
// ParseCriterion
//	Syntax:  criterion -> sortKey | expression
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseCriterion() {
	if (ParseSortKey())
		return true;
	return ParseExpression();
}

///////////////////////////////////////////////////////////////////////////////
// ParseExpression
//	Syntax:  expression -> boolTerm {'||' boolTerm}
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseExpression() {
	if (!ParseBoolTerm())
		return false;

	for (;;) {
		if (Symbol == OR) {
			GetSymbol();
			if (!ParseBoolTerm())
				throw Syntax("missing operand of ||",
					lex.position());
			Emit(OR);
		} else
			return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// ParseSortKey
//	Syntax:  sortKey -> ('max'|'min') variable
///////////////////////////////////////////////////////////////////////////////
bool
DrawingSurfaceFilter::ParseSortKey() {
	if (Symbol == MAX || Symbol == MIN) {
		EmitKey(Symbol);
		GetSymbol();
		if (FIRST_VAR < Symbol && Symbol < LAST_VAR) {
			EmitKey(Symbol);
			//
			// When sorting, eliminate visuals with a zero value
			// for the key.  This is hard to justify on grounds
			// of orthogonality, but it seems to yield the right
			// behavior (especially for ``min'').
			//
			Emit(Symbol);
			GetSymbol();
			return true;
		} else
			throw Syntax("missing variable name after sort key",
				lex.position());
	}

	return false;
} // DrawingSurfaceFilter::ParseSortKey


} // namespace GLEAN
