/**
 * @file texrect-many.c
 *
 * Tests whether the driver can support a full set of rectangle textures.
 *
 * (Prompted by a bug in R300 where the driver ran out of indirections).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 16*16;
	config.window_height = 11*16;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int NumTextures = 16;
static GLuint Textures[16];

static const GLubyte colors[7][4] = {
	{ 0, 0, 0, 255 },
	{ 255, 0, 0, 255 },
	{ 0, 255, 0, 255 },
	{ 0, 0, 255, 255 },
	{ 128, 0, 0, 128 },
	{ 0, 128, 0, 128 },
	{ 0, 0, 128, 128 }
};

static void ActiveTexture(int i)
{
	glActiveTexture(GL_TEXTURE0+i);
	glClientActiveTexture(GL_TEXTURE0+i);
}

static void DoFrame(void)
{
	int i;

	glClearColor(0.5, 0.5, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 0, 0);
		glVertex2f(0, 0);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 16, 0);
		glVertex2f(1, 0);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 16, 11);
		glVertex2f(1, 1);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 0, 11);
		glVertex2f(0, 1);
	glEnd();
}

static bool
DoTest(void)
{
	int x, y;
	bool pass = true;

	for(x = 0; x < NumTextures; ++x) {
		for(y = 0; y < 11; ++y) {
			float expected[4];
			int clr;
			int probe_x = (2*x+1) * piglit_width / 32;
			int probe_y = (2*y+1) * piglit_height / 22;

			clr = (x+y)%7;

			expected[0] = colors[clr][0] / 255.0;
			expected[1] = colors[clr][1] / 255.0;
			expected[2] = colors[clr][2] / 255.0;
			expected[3] = colors[clr][3] / 255.0;

			pass = pass && piglit_probe_pixel_rgba(probe_x, probe_y,
							       expected);
		}
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	int pass;

	DoFrame();
	pass = DoTest();

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	int i;
	int maxtextures;

	piglit_require_gl_version(13);

	piglit_require_extension("GL_ARB_texture_rectangle");

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxtextures);
	if (maxtextures < NumTextures)
		NumTextures = maxtextures;

	glGenTextures(NumTextures, Textures);
	for(i = 0; i < NumTextures; ++i) {
		GLubyte tex[11*16*4];
		GLubyte* p;
		int x, y;

		ActiveTexture(i);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, Textures[i]);

		p = tex;
		for(y = 0; y < 11; ++y) {
			for(x = 0; x < 16; ++x, p += 4) {
				if (x != i) {
					p[0] = p[1] = p[2] = p[3] = 255;
				} else {
					int clr = (x+y)%7;
					p[0] = colors[clr][0];
					p[1] = colors[clr][1];
					p[2] = colors[clr][2];
					p[3] = colors[clr][3];
				}
			}
		}

		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 16, 11, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, tex);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	piglit_ortho_projection(1.0, 1.0, GL_FALSE);
}
