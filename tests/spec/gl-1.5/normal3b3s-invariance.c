/*
 * Copyright (C) 2014 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * Test GLbyte[3] and GLshort[3] normal vectors and OpenGL invariance.
 *
 * We draw a lit, curved surface in two passes.  The first pass draws the
 * surface with a blue material and GLfloat[3] normal vectors.  The second
 * pass draws the surface with a green material and GLbyte[3] (or GLshort[3])
 * normal vectors.  The second pass uses glDepthFunc(GL_EQUAL) and additive
 * blending.  So the result should be a cyan surface (blue + green).
 *
 * If OpenGL uses different vertex transformation paths for the GLfloat[3]
 * vs. GLbyte[3] vs. GLshort[3] normal vectors we may get different vertex
 * positions, and different fragment Z values, and an unexpected surface color.
 *
 * Note: we use vertex buffers/arrays and not glNormal3b/3s since the later
 * might convert its parameters to floats.
 *
 * This test hits a VMware svga3d driver issue.
 *
 * Brian Paul
 * 11 April 2014
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 15;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH;
PIGLIT_GL_TEST_CONFIG_END


struct vertex {
	GLfloat pos[3];   /* position */
	GLfloat nf[3];    /* float normal */
	GLshort ns[3];    /* short normal */
	GLbyte nb[3];     /* byte normal */
};


static GLuint NumSections = 60;

static GLenum NormalType = GL_BYTE;

static const float blue[4] = { 0, 0, 1, 0 };
static const float green[4] = { 0, 1, 0, 0 };
static const float black[4] = { 0, 0, 0, 0 };


static GLbyte
float_to_byte(float v)
{
	return (GLbyte) (v * 127.0);
}


static GLshort
float_to_short(float v)
{
	return (GLshort) (v * 32767.0);
}

	
/**
 * Setup a VBO for a curved surface with vertex positions, float normals
 * and byte normals.
 */
static void
setup_vbo(void)
{
	const float radius = 20.0;
	int i, j;
	GLuint vbo;
	struct vertex *vbo_data;
	const unsigned num_verts = (NumSections + 1) * 2;
	const unsigned vbo_size = sizeof(struct vertex) * num_verts;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vbo_size, NULL, GL_STATIC_DRAW);
	vbo_data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	for (i = j = 0; i <= NumSections; i++) {
		float a = (float) i / (NumSections) * M_PI * 2.0;
		float x = cos(a);
		float z = sin(a);

		vbo_data[j].pos[0] = radius * x;
		vbo_data[j].pos[1] = -10.0;
		vbo_data[j].pos[2] = radius * z;
		vbo_data[j].nf[0] = x;
		vbo_data[j].nf[1] = 0;
		vbo_data[j].nf[2] = z;
		vbo_data[j].nb[0] = float_to_byte(x);
		vbo_data[j].nb[1] = 0;
		vbo_data[j].nb[2] = float_to_byte(z);
		vbo_data[j].ns[0] = float_to_short(x);
		vbo_data[j].ns[1] = 0;
		vbo_data[j].ns[2] = float_to_short(z);
		j++;

		vbo_data[j].pos[0] = radius * x;
		vbo_data[j].pos[1] = 10.0;
		vbo_data[j].pos[2] = radius * z;
		vbo_data[j].nf[0] = x;
		vbo_data[j].nf[1] = 0;
		vbo_data[j].nf[2] = z;
		vbo_data[j].nb[0] = float_to_byte(x);
		vbo_data[j].nb[1] = 0;
		vbo_data[j].nb[2] = float_to_byte(z);
		vbo_data[j].ns[0] = float_to_short(x);
		vbo_data[j].ns[1] = 0;
		vbo_data[j].ns[2] = float_to_short(z);
		j++;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
}


static void
draw_vbo(GLenum normal_type)
{
	const unsigned num_verts = (NumSections + 1) * 2;

	glVertexPointer(3, GL_FLOAT, sizeof(struct vertex),
			(void *) offsetof(struct vertex, pos));

	if (normal_type == GL_BYTE) {
		glNormalPointer(GL_BYTE, sizeof(struct vertex),
				(void *) offsetof(struct vertex, nb));
	}
	else if (normal_type == GL_SHORT) {
		glNormalPointer(GL_SHORT, sizeof(struct vertex),
				(void *) offsetof(struct vertex, ns));
	}
	else {
		assert(normal_type == GL_FLOAT);
		glNormalPointer(GL_FLOAT, sizeof(struct vertex),
				(void *) offsetof(struct vertex, nf));
	}

	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_NORMAL_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);
}


static bool
draw(void)
{
	GLfloat pix[3];
	bool pass;

	glViewport(0, 0, piglit_width, piglit_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1, 1, -1, 1, 2, 200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, -5, -80);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);

	glPushMatrix();
	glRotatef(-25, 1, 0, 0);

	/* Draw blue base color with glDepthFunc(<=) */
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
	glDepthFunc(GL_LEQUAL);
	draw_vbo(GL_FLOAT);

	/* Draw green highlight color with glDepthFunc(==).
	 * We should generate fragments with the same Z value as the
	 * first pass.
	 */
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glDepthFunc(GL_EQUAL);
	draw_vbo(NormalType);

	glPopMatrix();

	/* Probe: the center pixel should cyan (blue + green) */
	glReadPixels(piglit_width / 2, piglit_height / 2, 1, 1,
		     GL_RGB, GL_FLOAT, pix);
	pass = (pix[0] == 0.0 &&
		pix[1] >= 0.75 &&
		pix[2] >= 0.75);
	if (!pass) {
		printf("Expected (r=0, g>=0.75, b>=0.75), found (%g, %g, %g)\n",
		       pix[0], pix[1], pix[2]);
	}

	piglit_present_results();

	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = draw();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	if (argc > 1) {
		if (strcmp(argv[1], "GL_BYTE") == 0) {
			NormalType = GL_BYTE;
		}
		else if (strcmp(argv[1], "GL_SHORT") == 0) {
			NormalType = GL_SHORT;
		}
		else {
			printf("Expected argument GL_BYTE or GL_SHORT\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	}
			

	glMaterialfv(GL_FRONT, GL_EMISSION, black);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
	glMaterialfv(GL_FRONT, GL_SPECULAR, black);
	glMaterialf(GL_FRONT, GL_SHININESS, 5);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	glBlendFunc(GL_ONE, GL_ONE);

	setup_vbo();
}
