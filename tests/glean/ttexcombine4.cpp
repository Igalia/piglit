// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2009  VMware, Inc. All Rights Reserved.
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
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL VMWARE BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT


// Test GL_NV_texture_env_combine4
// Generate some random combiner state and colors, compute the expected
// color, then render with the combiner state and compare the results.
// Only one texture unit is tested and not all possible combiner terms
// are exercised.
//
// Brian Paul
// 23 Jan 2009


#define GL_GLEXT_PROTOTYPES

#include <cassert>
#include <cmath>
#include <cstring>
#include "ttexcombine4.h"
#include "timer.h"


namespace GLEAN {

TexCombine4Result::TexCombine4Result()
{
	pass = true;
}


// generate random combiner state
void
TexCombine4Test::generate_state(struct combine_state &state)
{
	int i;

	if (rand.next() > 0.5f)
		state.CombineMode = GL_ADD;
	else
		state.CombineMode = GL_ADD_SIGNED_EXT;

	for (i = 0; i < 4; i++) {
		int src = int(rand.next() * 4.0);
		switch (src) {
		case 0:
			state.Source[i] = GL_ZERO;
			break;
		case 1:
			state.Source[i] = GL_TEXTURE;
			break;
		case 2:
			state.Source[i] = GL_CONSTANT_EXT;
			break;
		default:
			state.Source[i] = GL_PRIMARY_COLOR_EXT;
			break;
		}

		if (rand.next() > 0.5f) {
			state.OperandRGB[i] = GL_SRC_COLOR;
			state.OperandA[i] = GL_SRC_ALPHA;
		}
		else {
			state.OperandRGB[i] = GL_ONE_MINUS_SRC_COLOR;
			state.OperandA[i] = GL_ONE_MINUS_SRC_ALPHA;
		}
	}

	state.PrimaryColor[0] = rand.next();
	state.PrimaryColor[1] = rand.next();
	state.PrimaryColor[2] = rand.next();
	state.PrimaryColor[3] = rand.next();

	state.ConstantColor[0] = rand.next();
	state.ConstantColor[1] = rand.next();
	state.ConstantColor[2] = rand.next();
	state.ConstantColor[3] = rand.next();

	state.TextureColor[0] = rand.next();
	state.TextureColor[1] = rand.next();
	state.TextureColor[2] = rand.next();
	state.TextureColor[3] = rand.next();
}


// compute expected final color
void
TexCombine4Test::evaluate_state(const struct combine_state &state,
				GLfloat result[4])
{
	GLfloat arg[4][4];
	int i;

	// setup terms
	for (i = 0; i < 4; i++) {
		switch (state.Source[i]) {
		case GL_ZERO:
			arg[i][0] = arg[i][1] = arg[i][2] = arg[i][3] = 0.0f;
			break;
		case GL_PRIMARY_COLOR_EXT:
			arg[i][0] = state.PrimaryColor[0];
			arg[i][1] = state.PrimaryColor[1];
			arg[i][2] = state.PrimaryColor[2];
			arg[i][3] = state.PrimaryColor[3];
			break;
		case GL_CONSTANT_EXT:
			arg[i][0] = state.ConstantColor[0];
			arg[i][1] = state.ConstantColor[1];
			arg[i][2] = state.ConstantColor[2];
			arg[i][3] = state.ConstantColor[3];
			break;
		case GL_TEXTURE:
			arg[i][0] = state.TextureColor[0];
			arg[i][1] = state.TextureColor[1];
			arg[i][2] = state.TextureColor[2];
			arg[i][3] = state.TextureColor[3];
			break;
		default:
			assert(0);
		}

		switch (state.OperandRGB[i]) {
		case GL_SRC_COLOR:
			// nop
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			arg[i][0] = 1.0f - arg[i][0];
			arg[i][1] = 1.0f - arg[i][1];
			arg[i][2] = 1.0f - arg[i][2];
			arg[i][3] = 1.0f - arg[i][3];
			break;
		default:
			assert(0);
		}
	}

	// combine terms, loop over color channels
	for (i = 0; i < 4; i++) {
		result[i] = arg[0][i] * arg[1][i] + arg[2][i] * arg[3][i];
		if (state.CombineMode == GL_ADD_SIGNED_EXT)
			result[i] -= 0.5f;
		if (result[i] < 0.0f)
			result[i] = 0.0f;
		else if (result[i] > 1.0f)
			result[i] = 1.0f;
	}
}


// render quad with given combiner state and return resulting color
// return false if GL error is detected, true otherwise.
bool
TexCombine4Test::render_state(const struct combine_state &state,
			      GLfloat result[4])
{
	if (glGetError()) {
		reportError("GL error detected before setting combiner state.");
		return false;
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, state.CombineMode);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, state.CombineMode);

	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, state.Source[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, state.Source[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, state.Source[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, state.Source[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, state.Source[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, state.Source[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, state.Source[3]);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, state.Source[3]);

	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, state.OperandRGB[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, state.OperandA[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, state.OperandRGB[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, state.OperandA[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, state.OperandRGB[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, state.OperandA[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, state.OperandRGB[3]);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, state.OperandA[3]);

	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, state.ConstantColor);

	if (glGetError()) {
		reportError("GL error generated by combiner state.");
		return false;
	}

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
		     GL_RGBA, GL_FLOAT, state.TextureColor);

	glColor4fv(state.PrimaryColor);

	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-1.0, -1.0);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(1.0, -1.0);
	glTexCoord2f(1.0, 1.0);
	glVertex2f( 1.0,  1.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(-1.0,  1.0);
	glEnd();

	glReadPixels(WINDOW_SIZE / 2, WINDOW_SIZE / 2, 1, 1,
		     GL_RGBA, GL_FLOAT, result);

	return true;
}


void
TexCombine4Test::report_state(const struct combine_state &state)
{
	env->log << "\tCurrent GL state:\n";

	if (state.CombineMode == GL_ADD)
		env->log << "\t\tCOMBINE = GL_ADD\n";
	else
		env->log << "\t\tCOMBINE = GL_ADD_SIGNED_EXT\n";

	for (int i = 0; i < 4; i++) {
		env->log << "\t\t" << "SOURCE" << i << ": ";
		switch (state.Source[i]) {
		case GL_ZERO:
			env->log << "GL_ZERO\n";
			break;
		case GL_TEXTURE:
			env->log << "GL_TEXTURE\n";
			break;
		case GL_CONSTANT_EXT:
			env->log << "GL_CONSTANT\n";
			break;
		case GL_PRIMARY_COLOR_EXT:
			env->log << "GL_PRIMARY_COLOR\n";
			break;
		default:
			assert(0);
		}

		env->log << "\t\t" << "OPERAND" << i << "_RGB: ";
		switch (state.OperandRGB[i]) {
		case GL_SRC_COLOR:
			env->log << "GL_SRC_COLOR\n";
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			env->log << "GL_ONE_MINUS_SRC_COLOR\n";
			break;
		default:
			assert(0);
		}

		env->log << "\t\t" << "OPERAND" << i << "_ALPHA: ";
		switch (state.OperandRGB[i]) {
		case GL_SRC_COLOR:
			env->log << "GL_SRC_ALPHA\n";
			break;
		case GL_ONE_MINUS_SRC_COLOR:
			env->log << "GL_ONE_MINUS_SRC_ALPHA\n";
			break;
		default:
			assert(0);
		}

	}

	char str[100];
	sprintf(str, "%.3f, %.3f, %.3f, %.3f",
		state.PrimaryColor[0], state.PrimaryColor[1],
		state.PrimaryColor[2], state.PrimaryColor[3]);
	env->log << "\t\tPrimary Color: " << str << "\n";
	sprintf(str, "%.3f, %.3f, %.3f, %.3f",
		state.ConstantColor[0], state.ConstantColor[1],
		state.ConstantColor[2], state.ConstantColor[3]);
	env->log << "\t\tConstant Color: " << str << "\n";
	sprintf(str, "%.3f, %.3f, %.3f, %.3f",
		state.TextureColor[0], state.TextureColor[1],
			state.TextureColor[2], state.TextureColor[3]);
	env->log << "\t\tTexture Color: " << str << "\n";
}


void
TexCombine4Test::reportError(const char *msg)
{
	env->log << name << ": Error: " << msg << "\n";
}


void
TexCombine4Test::runOne(TexCombine4Result &r, Window &w)
{
	(void) w;  // silence warning

	rand = RandomDouble(42);  // init random number generator

	const float err = 0.05;  // xxx compute something better

	for (int i = 0; i < NUM_TESTS; i++) {
		combine_state state;
		GLfloat expected[4], actual[4];

		//env->log << "\t iteration " << i << "\n";

		generate_state(state);

		evaluate_state(state, expected);

		if (!render_state(state, actual)) {
			r.pass = false;
			return;
		}

		if (fabs(expected[0] - actual[0]) > err ||
		    fabs(expected[1] - actual[1]) > err ||
		    fabs(expected[2] - actual[2]) > err) {
			char str[100];
			env->log << name << ": Error: GL_NV_texure_env_combine4 failed\n";
			report_state(state);
			env->log << "\tResults:\n";
			sprintf(str, "%.3f, %.3f, %.3f, %.3f",
				expected[0], expected[1],
				expected[2], expected[3]);
			env->log << "\t\tExpected color: " << str << "\n";
			sprintf(str, "%.3f, %.3f, %.3f, %.3f",
				actual[0], actual[1],
				actual[2], actual[3]);
			env->log << "\t\tRendered color: " << str << "\n";
			r.pass = false;
			return;
		}
	}

	r.pass = true;
}


void
TexCombine4Test::logOne(TexCombine4Result &r)
{
	logPassFail(r);
	logConcise(r);
}


void
TexCombine4Test::compareOne(TexCombine4Result &oldR,
			     TexCombine4Result &newR)
{
	comparePassFail(oldR, newR);
}


void
TexCombine4Result::putresults(ostream &s) const
{
	if (pass) {
		s << "PASS\n";
	}
	else {
		s << "FAIL\n";
	}
}


bool
TexCombine4Result::getresults(istream &s)
{
	char result[1000];
	s >> result;

	if (strcmp(result, "FAIL") == 0) {
		pass = false;
	}
	else {
		pass = true;
	}
	return s.good();
}


// The test object itself:
TexCombine4Test texCombine4Test("texCombine4", "window, rgb",
				"GL_NV_texture_env_combine4, GL_EXT_texture_env_combine",
				"Test the GL_NV_texture_env_combine4 extension.\n");



} // namespace GLEAN


