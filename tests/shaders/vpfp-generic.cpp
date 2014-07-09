/*
 * Copyright (c) 2009 Nicolai Haehnle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Generic ARB_vertex_program + ARB_fragment_program test,
 * to test ALU / compiler correctness.
 * Takes an input file of the following form:
 *
 * !!ARBvp1.0
 * vertex program (should use OPTION ARB_position_invariant)
 * END
 *
 * !!ARBfp1.0
 * fragment program
 * END
 *
 * !!test
 * parameter x y z w
 * parameter x y z w
 * ...
 * expected x y z w
 *
 * Arbitrarily many test sections can be given, and in each section,
 * arbitrarily many parameters are possible.
 *
 * Also supports NV_vertex_program by using !!VP1.0 instead of !!ARBvp1.0.
 * For NV_vertex_program, parameters can be set by
 *  NVparameter[<id>] <x> <y> <z> <w>
 * Note that the model-view-projection matrix is tracked in parameters
 * [0..3].
 *
 * Keep in mind that the ARB_vertex_program and NV_vertex_program tokens and
 * entrypoints largely overlap.  If an implementation supports both, it is
 * perfectly legal to pass an "VP1.0" program to the ARB entrypoint and an
 * "ARBvp1.0" program to the NV entrypoint.  On most implementations the
 * entrypoints alias.  Also notice that \c GL_VERTEX_PROGRAM_NV and
 * \c GL_VERTEX_PROGRAM_ARB have the same value.
 */

#include <string>
#include <sstream>
#include <vector>

extern "C" {
#include "piglit-util-gl.h"
}

using namespace std;



struct TestParameter {
	TestParameter() {}
	virtual ~TestParameter() {}
	virtual void setup() = 0;
	virtual void teardown() { }

private:
	TestParameter(const TestParameter&); // don't copy
	TestParameter& operator=(const TestParameter&);
};

struct ParameterTexcoord : TestParameter {
	ParameterTexcoord(int _tcu, GLfloat v[4]) : tcu(_tcu) {
		memcpy(texcoords, v, sizeof(GLfloat)*4);
	}

	void setup() {
		glMultiTexCoord4fv(GL_TEXTURE0+tcu, texcoords);
	}

	int tcu;
	GLfloat texcoords[4];
};

struct ParameterLocal : TestParameter {
	ParameterLocal(GLenum _target, int _index, GLfloat v[4]) {
		target = _target;
		index = _index;
		memcpy(data, v, sizeof(GLfloat)*4);
	}

	void setup() {
		glProgramLocalParameter4fvARB(target, index, data);
	}

protected:
	GLenum target;
	int index;
	GLfloat data[4];
};

struct ParameterEnv : ParameterLocal {
	ParameterEnv(GLenum _target, int _index, GLfloat v[4])
		: ParameterLocal(_target, _index, v)
	{
		/* empty */
	}

	void setup() {
		glProgramEnvParameter4fvARB(target, index, data);
	}
};

struct Test {
	Test(const std::string& _name) : name(_name) {
		expected[0] = expected[1] = expected[2] = expected[3] = 0;
	}
	~Test() {
		for(vector<TestParameter*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
			delete *it;
	}

	bool run();
	void readline(const char* filename, int linenum, char* line);

	string name;
	vector<TestParameter*> parameters;
	GLfloat expected[4];

private:
	Test(const Test&); // don't copy
	Test& operator=(const Test&);
};

struct TestGroup {
	TestGroup() {
		nv_vertex_program = false;
	}
	~TestGroup() {
		for(vector<Test*>::iterator ptest = tests.begin(); ptest != tests.end(); ++ptest)
			delete *ptest;
	}

	bool run();
	void read(const char* filename);

	string vertex_program_code;
	string fragment_program_code;
	bool nv_vertex_program;
	vector<Test*> tests;

private:
	TestGroup(const TestGroup&); // don't copy
	TestGroup& operator=(const TestGroup&);
};


PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char* Filename = 0;
static TestGroup tests;



bool Test::run()
{
	glClearColor(
		expected[0] > 0.5 ? 0.0 : 1.0,
		expected[1] > 0.5 ? 0.0 : 1.0,
		expected[2] > 0.5 ? 0.0 : 1.0,
		expected[3] > 0.5 ? 0.0 : 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for(vector<TestParameter*>::iterator pparm = parameters.begin(); pparm != parameters.end(); ++pparm)
		(*pparm)->setup();

	glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(1, 0);
		glVertex2f(1, 1);
		glVertex2f(0, 1);
	glEnd();

	for(vector<TestParameter*>::iterator pparm = parameters.begin(); pparm != parameters.end(); ++pparm)
		(*pparm)->teardown();

	if (!piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected)) {
		fprintf(stderr, "Test %s failed\n", name.c_str());
		return false;
	}
	return true;
}


bool TestGroup::run()
{
	bool success = true;
	GLuint fragprog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, fragment_program_code.c_str());

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragprog);

	GLuint vertprog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB,
						 vertex_program_code.c_str());

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertprog);

	if (nv_vertex_program) {
		glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
	}

	for(vector<Test*>::iterator it = tests.begin(); it != tests.end(); ++it)
		success = (*it)->run() && success;

	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);

	glFlush();

	return success;
}


#if 0
static void expect(FILE* filp, const char* str)
{
	char buf[41];
	fscanf(filp, "%40s", buf);
	if (strcmp(buf, str)) {
		fprintf(stderr, "Expected '%s', got '%s'\n", str, buf);
		exit(-1);
	}
}

static GLfloat* readfloatarray(FILE* filp, int count)
{
	GLfloat* dest = (GLfloat*)malloc(sizeof(GLfloat)*count);
	int i;

	for(i = 0; i < count; ++i)
		fscanf(filp, "%f", &dest[i]);

	return dest;
}
#endif

void Test::readline(const char* filename, int linenum, char* line)
{
	char* p = strchr(line, ' ');
	if (!p) {
		fprintf(stderr, "%s:%i: malformed test line\n", filename, linenum);
		piglit_report_result(PIGLIT_FAIL);
	}

	*p++ = 0;

	GLfloat params[4];
	sscanf(p, "%f %f %f %f", &params[0], &params[1], &params[2], &params[3]);

	if (!strcmp(line, "expected")) {
		memcpy(expected, params, sizeof(GLfloat)*4);
	} else if (!strncmp(line, "texcoord[", 9)) {
		parameters.push_back(new ParameterTexcoord(atoi(line+9), params));
	} else if (!strncmp(line, "vertex.local[", 13)) {
		parameters.push_back(new ParameterLocal(GL_VERTEX_PROGRAM_ARB, atoi(line+13), params));
	} else if (!strncmp(line, "vertex.environment[", 19)) {
		parameters.push_back(new ParameterEnv(GL_VERTEX_PROGRAM_ARB, atoi(line+19), params));
	} else if (!strncmp(line, "fragment.local[", 15)) {
		parameters.push_back(new ParameterLocal(GL_FRAGMENT_PROGRAM_ARB, atoi(line+15), params));
	} else if (!strncmp(line, "NVparameter[", 12)) {
		parameters.push_back(new ParameterEnv(GL_VERTEX_PROGRAM_NV, atoi(line+12), params));
	} else {
		fprintf(stderr, "%s:%i: unknown parameters %s\n", filename, linenum, line);
		piglit_report_result(PIGLIT_FAIL);
	}
}


enum ReadState {
	None = 0,
	ReadVertexProgram,
	ReadFragmentProgram,
	ReadTest
};

void TestGroup::read(const char* filename)
{
	FILE* filp = fopen(filename, "rt");
	char buf[256];
	int linenum = 0;

	if (!filp) {
		fprintf(stderr, "Failed to read test data: %s\n", filename);
		piglit_report_result(PIGLIT_FAIL);
	}

	ReadState state = None;
	Test* test = 0;

	while(fgets(buf, sizeof(buf), filp)) {
		linenum++;

		char* p = buf;
		while(isspace(*p))
			p++;

		if (!*p || *p == ';')
			continue;

		if (!strncmp(p, "!!", 2)) {
			p += 2;
			if (!strncmp(p, "ARBvp", 5)) {
				vertex_program_code = "!!";
				vertex_program_code += p;
				state = ReadVertexProgram;
			} else if (!strncmp(p, "VP", 2)) {
				vertex_program_code = "!!";
				vertex_program_code += p;
				nv_vertex_program = true;
				state = ReadVertexProgram;
			} else if (!strncmp(p, "ARBfp", 5)) {
				fragment_program_code = "!!";
				fragment_program_code += p;
				state = ReadFragmentProgram;
			} else if (!strncmp(p, "test", 4)) {
				stringstream name;
				name << filename << ":" << linenum;
				test = new Test(name.str());
				tests.push_back(test);
				state = ReadTest;
			} else {
				fprintf(stderr, "%s:%i: unknown %s\n", filename, linenum, p);
				piglit_report_result(PIGLIT_FAIL);
			}
		} else {
			switch(state) {
			case ReadVertexProgram:
				vertex_program_code += p;
				break;
			case ReadFragmentProgram:
				fragment_program_code += p;
				break;
			case ReadTest:
				test->readline(filename, linenum, p);
				break;
			default:
				fprintf(stderr, "%s:%i: unexpected: %s\n", filename, linenum, p);
				piglit_report_result(PIGLIT_FAIL);
			}
		}
	}

	fclose(filp);
}


extern "C" piglit_result piglit_display(void)
{
	return tests.run() ? PIGLIT_PASS : PIGLIT_FAIL;
}

extern "C" void piglit_init(int argc, char **argv)
{
	int i;
	const char *fp;

	for(i = 1; i < argc; ++i) {
		if (!Filename)
			Filename = argv[i];
	}
	if (!Filename) {
		fprintf(stderr, "Need to give a testcase file\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	tests.read(Filename);

	fp = tests.fragment_program_code.c_str();

	if (piglit_get_gl_version() < 13) {
		printf("Requires OpenGL 1.3\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	if (strstr(fp, "OPTION ARB_fragment_coord_origin_upper_left") ||
	    strstr(fp, "OPTION ARB_fragment_coord_pixel_center_integer")) {
		piglit_require_extension("GL_ARB_fragment_coord_conventions");
	}

	piglit_require_fragment_program();
	piglit_require_vertex_program();
	if (tests.nv_vertex_program)
		piglit_require_extension("GL_NV_vertex_program");

	piglit_ortho_projection(1.0, 1.0, GL_FALSE);
}
