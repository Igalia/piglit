/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file clipping-transforms.c
 *
 * This test verifies that clip planes are transformed using the
 * correct matrices, at the correct times.
 *
 * The transformations affecting clipping in fixed functionality mode
 * (with no vertex shader) are described in the OpenGL 2.1 spec,
 * section 2.12 ("Clipping"):
 *
 *     "A client-defined clip plane is specified with
 *
 *         void ClipPlane( enum p, double eqn[4] );
 *
 *     ... eqn is an array of four double-precision floating-point
 *     values. These are the coefficients of a plane equation in
 *     object coordinates: p1 , p2 , p3 , and p4 (in that order). The
 *     inverse of the current model-view matrix is applied to these
 *     coefficients, at the time they are specified, yielding
 *
 *         (p1' p2' p3' p4') = (p1 p2 p3 p4) M^-1
 *
 *     (where M is the current model-view matrix; the resulting plane
 *     equation is undefined if M is singular and may be inaccurate if
 *     M is poorly-conditioned) to obtain the plane equation
 *     coefficients in eye coordinates.  All points with eye
 *     coordinates (xe ye ze we)^T that satisfy
 *
 *         (p1' p2' p3' p4') (xe ye ze we)^T >= 0
 *
 *     lie in the half-space defined by the plane; points that do not
 *     satisfy this condition do not lie in the half-space."
 *
 * Thus, the clip planes should be modified by the value of the
 * model-view matrix at the time clip planes are specified; the value
 * of the model-view transformation at drawing time should have no
 * effect on which part of the scene is clipped.
 *
 * The projection matrix, on the other hand, should be the opposite:
 * its value at the time clip planes are specified should have no
 * effect, but its value at drawing time should determine where on the
 * screen clipping takes place (since clipping is performed on eye
 * coordinates, before the perspective matrix is applied).
 *
 * The transformations affecting clipping when a vertex shader is
 * present can be inferred from the text that follows:
 *
 *     "When a vertex shader is active, the vector (xe ye ze we)^T is
 *     no longer computed. Instead, the value of the gl_ClipVertex
 *     built-in variable is used in its place."
 *
 * So, as before, the model-view matrix affects clip planes at the
 * time they are specified, but not at draw time.  However, the
 * projection matrix no longer necessarily has an effect; instead, the
 * place on the screen where clipping takes place is determined by the
 * relationship between the values of gl_Position and gl_ClipVertex
 * that are output by the vertex shader.
 *
 * It's also possible that the vertex shader might not store a value
 * in gl_ClipVertex at all; what happens in this case is less clear.
 * According to the GL 2.1 spec (from the same section):
 *
 *     "If gl ClipVertex is not written by the vertex shader, its
 *     value is undefined, which implies that the results of clipping
 *     to any client-defined clip planes are also undefined."
 *
 * The GL 3.0 spec says the same thing, and the GLSL 1.10 and 1.20
 * specs have compatible language (from section 7.1: Vertex Shader
 * Special Variables):
 *
 *     "If gl_PointSize or gl_ClipVertex are not written to, their
 *     values are undefined."
 *
 * However, the GLSL 1.30 spec says:
 *
 *     "If a linked set of shaders forming the vertex stage contains
 *     no static write to gl_ClipVertex or gl_ClipDistance, but the
 *     application has requested clipping against user clip planes
 *     through the API, then the coordinate written to gl_Position is
 *     used for comparison against the user clip planes."
 *
 * So, if GLSL 1.30 is to be believed, if the vertex shader does not
 * write to gl_Position, then the place on the screen where clipping
 * takes place is determined exclusively by the plane equation (p1'
 * p2' p3' p4').  No further transformation is applied.
 *
 * Note that strictly speaking, this doesn't contradict any of the
 * other specs, since a conformant implementation may do anything it
 * desires when behavior is "undefined", including clipping based on
 * gl_Position.  Since this behavior is only specified in GLSL 1.30,
 * we include a "#version 130" directive in the shader when testing
 * it.
 *
 *
 * The test operates by constructing four clip plane equations which
 * are only satisfied by points within a small square region near (1,
 * 0).  Setting all matrices to the identity matrix, and setting
 * gl_Position == gl_ClipVertex == gl_Vertex, it draws a large square,
 * large enough to cover the entire window, and then probes the
 * resulting image to determine where pixels were actually drawn; due
 * to clipping, they should be drawn only near (1, 0).
 *
 * Then it performs a 20 degree rotation in each of the following ways
 * in turn, leaving all other transformations as the identity
 * transformation:
 * - Using the model-view matrix at the time clip planes are specified
 * - Using the projection matrix at the time clip planes are specified
 * - Using the model-view matrix at the time of drawing
 * - Using the projection matrix at the time of drawing
 * - Using the vertex shader to rotate gl_Position with respect to gl_Vertex
 * - Using the vertex shader to rotate gl_ClipVertex with respect to gl_Vertex
 *
 * In each case it probes the resulting image to determine where
 * pixels were actually drawn, and compares the result to the expected
 * behavior from the spec.
 *
 *
 * The test may be run in one of four modes, chosen with a single
 * command line argument:
 * - "fixed": test using fixed functionality (no vertex shader)
 * - "arb": test using GL_ARB_vertex_program extension (see below)
 * - "pos": test using a vertex shader that sets gl_Position only
 * - "pos_clipvert": test using a vertex shader that sets gl_Position first,
 *                   then gl_ClipVertex
 * - "clipvert_pos": test using a vertex shader that sets gl_ClipVertex first,
 *                   then gl_Position
 *
 * The reason for distinguishing between "pos_clipvert" and
 * "clipvert_pos" is that in the present Mesa implementation, the
 * variables gl_Position and gl_ClipVertex are aliases of each other,
 * so the order in which values are stored into these two variables
 * may affect shader behavior.
 *
 * Note: "arb" mode tests using an ARB vertex program, as defined in
 * the GL_ARB_vertex_program extension.  From the extension spec:
 *
 *     "User-defined clipping is not supported in standard vertex
 *     program mode.  User-defined clipping support will be provided
 *     for programs that use the "position invariant" option, where
 *     all vertex transformation operations are performed by the
 *     fixed-function pipeline."
 *
 * The strong implication seems to be that for ARB vertex programs
 * that use the "position invariant" option, clipping should behave as
 * it does in fixed function mode.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLint position_angle_loc;
GLint clipVertex_angle_loc;
bool use_ff = false;
bool use_arb = false;
bool use_glsl = false;
bool use_clip_vertex = false;
bool use_glsl_130 = false;

/**
 * GLSL code used to set gl_Position and/or gl_ClipVertex in the
 * vertex shader.
 */
char *setters;

void
setup_glsl_programs()
{
	GLuint prog;

	char vert[4096];
	char frag[4096];
	char *version_directive;

	if (use_glsl_130) {
		version_directive = "#version 130";
	} else {
		version_directive = "";
	}

	sprintf(vert,
		"%s\n"
		"uniform float position_angle;\n"
		"uniform float clipVertex_angle;\n"
		"mat4 rotate(float angle)\n"
		"{\n"
		"  angle = radians(angle);\n"
		"  return mat4( cos(angle), sin(angle), 0.0, 0.0,\n"
		"              -sin(angle), cos(angle), 0.0, 0.0,\n"
		"                      0.0,        0.0, 1.0, 0.0,\n"
		"                      0.0,        0.0, 0.0, 1.0);\n"
		"}\n"
		"void main()\n"
		"{\n"
		"%s\n"
		"}",
		version_directive, setters);
	sprintf(frag,
		"%s\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}",
		version_directive);

	prog = piglit_build_simple_program(vert, frag);
	glUseProgram(prog);
	position_angle_loc = glGetUniformLocation(prog, "position_angle");
	if (use_clip_vertex) {
		clipVertex_angle_loc =
			glGetUniformLocation(prog, "clipVertex_angle");
	}
}

void
setup_arb_program()
{
	char vert[] =
		"!!ARBvp1.0\n"
		"OPTION ARB_position_invariant;\n"
		"MOV result.color, { 1.0, 1.0, 1.0, 1.0 };"
		"END";
	GLuint vert_prog;

	glGenProgramsARB(1, &vert_prog);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vert_prog);
	glProgramStringARB(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
			   strlen(vert), vert);
	glEnable(GL_VERTEX_PROGRAM_ARB);
}

void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <mode>\n"
	       "  where <mode> is one of:\n"
	       "    fixed\n"
	       "    arb\n"
	       "    pos\n"
	       "    pos_clipvert\n"
	       "    clipvert_pos\n", prog_name);
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "fixed") == 0) {
		use_ff = true;
	} else if (strcmp(argv[1], "arb") == 0) {
		use_arb = true;
	} else if (strcmp(argv[1], "pos") == 0) {
		use_glsl = true;
		setters = "  gl_Position = rotate(position_angle) * gl_Vertex;\n";
		use_glsl_130 = true;
	} else if (strcmp(argv[1], "pos_clipvert") == 0) {
		use_glsl = true;
		setters =
			"  gl_Position = rotate(position_angle) * gl_Vertex;\n"
			"  gl_ClipVertex = rotate(clipVertex_angle) * gl_Vertex;\n";
		use_clip_vertex = true;
	} else if (strcmp(argv[1], "clipvert_pos") == 0) {
		use_glsl = true;
		setters =
			"  gl_ClipVertex = rotate(clipVertex_angle) * gl_Vertex;\n"
			"  gl_Position = rotate(position_angle) * gl_Vertex;\n";
		use_clip_vertex = true;
	} else {
		print_usage_and_exit(argv[0]);
	}

	if (use_arb) {
		piglit_require_extension("GL_ARB_vertex_program");
		setup_arb_program();
	} else if (use_glsl) {
		piglit_require_GLSL();
		piglit_require_GLSL_version(use_glsl_130 ? 130 : 110);
		setup_glsl_programs();
	}
}

void
setup_clip_plane(int plane, float p1, float p2, float p3, float p4)
{
	double eqn[4] = { p1, p2, p3, p4 };
	glClipPlane(GL_CLIP_PLANE0 + plane, eqn);
}

bool
measure_effects(char *desc, int mc, int pc, int md, int pd, int expected)
{
	float size = 0.1;
	float dist = 1.0 - size/2;

	int angle;

	printf("Measuring %s: ", desc);

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(mc, 0, 0, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glRotatef(pc, 0, 0, 1);

	setup_clip_plane(0,  1.0,  0.0, 0.0, size-1.0); /* x > 1.0-size */
	setup_clip_plane(1, -1.0,  0.0, 0.0,      1.0); /* x < 1.0 */
	setup_clip_plane(2,  0.0,  1.0, 0.0,   size/2); /* y > -size/2 */
	setup_clip_plane(3,  0.0, -1.0, 0.0,   size/2); /* y < size/2 */
	glEnable(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE3);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(md, 0, 0, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glRotatef(pd, 0, 0, 1);

	piglit_draw_rect(-2, -2, 4, 4);

	for (angle = -180; angle < 180; angle += 10) {
		float angle_rad = angle * M_PI / 180.0;
		float xf = dist * cos(angle_rad);
		float yf = dist * sin(angle_rad);
		int x = (int) (0.5 + piglit_width * (xf + 1.0)/2.0);
		int y = (int) (0.5 + piglit_width * (yf + 1.0)/2.0);
		float found_color[4];
		glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, found_color);
		if (found_color[0] > 0.5) {
			if (angle == expected) {
				printf("OK (angle=%d)\n", angle);
				return true;
			} else {
				printf("FAIL (angle=%d, expected=%d)\n", angle,
				       expected);
				return false;
			}
		}
	}
	printf("FAIL (test rect not found, expected=%d)\n", expected);
	return false;
}

enum piglit_result
piglit_display()
{
	bool pass = true;

	if (use_glsl) {
		glUniform1f(position_angle_loc, 0.0);
		glUniform1f(clipVertex_angle_loc, 0.0);
	}

	/* Base behavior: no rotations, so the clipping planes should
	 * show up on screen at the coordinates where they were
	 * defined
	 */
	pass = measure_effects("base behavior", 0, 0, 0, 0, 0) && pass;

	/* A 20 degree rotation in the model-view matrix at the time
	 * clip planes are specified should result in a 20 degree
	 * rotation of where clipping takes effect.
	 */
	pass = measure_effects(
	    "effect of 20deg ModelView rotation while setting clip plane",
	    20, 0, 0, 0, 20) && pass;

	/* A 20 degree rotation in the projection matrix at the time
	 * clip planes are specified should have no effect.
	 */
	pass = measure_effects(
            "effect of 20deg Projection rotation while setting clip plane",
	    0, 20, 0, 0, 0) && pass;

	/* A 20 degree rotation in the model-view matrix at the time
	 * of drawing should have no effect.
	 */
	pass = measure_effects(
            "effect of 20deg ModelView rotation while drawing",
	    0, 0, 20, 0, 0) && pass;

	/* When using fixed functionality or an ARB position invariant
	 * program, a 20 degree rotation in the projection matrix at
	 * the time of drawing should result in a 20 degree rotation
	 * of where clipping takes effect when using fixed
	 * functionality.  When using a vertex shader, it should have
	 * no effect.
	 */
	pass = measure_effects(
            "effect of 20deg Projection rotation while drawing",
	    0, 0, 0, 20, use_ff || use_arb ? 20 : 0) && pass;

	if (use_glsl) {
		/* When a vertex shader sets gl_Position to be 20
		 * degrees rotated compared to gl_Vertex, and sets
		 * gl_ClipVertex to be equal to gl_Vertex, this should
		 * result in a 20 degree rotation of where clipping
		 * takes effect, because it causes gl_Position to be
		 * rotated 20 degrees with respect to gl_ClipVertex.
		 * However, when a vertex shader sets gl_Position and
		 * does not set gl_ClipVertex, there should be no
		 * effect, because the shader should behave as though
		 * it set gl_ClipVertex equal to gl_Position.
		 */
		glUniform1f(position_angle_loc, 20.0);
		pass = measure_effects(
		    "effect of 20deg rotation on gl_Position",
		    0, 0, 0, 0, use_clip_vertex ? 20 : 0) && pass;
		glUniform1f(position_angle_loc, 0.0);
	}

	if (use_clip_vertex) {
		/* When a vertex shader sets gl_Position to be equal
		 * to gl_Vertex, and sets gl_ClipVertex to be 20
		 * degrees rotated compared to gl_Vertex, this should
		 * result in a negative 20 degree rotation of where
		 * clipping takes effect, because it causes
		 * gl_Position to be rotated negative 20 degrees with
		 * respect to gl_ClipVertex.
		 */
		glUniform1f(clipVertex_angle_loc, 20.0);
		pass = measure_effects(
		    "effect of 20deg rotation on gl_ClipVertex",
		    0, 0, 0, 0, -20) && pass;
		glUniform1f(clipVertex_angle_loc, 0.0);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
