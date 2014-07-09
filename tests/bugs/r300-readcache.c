/**
 * @file r300-readcache.c
 *
 * Test case for an odd problem in Radeon on-chip readcache.
 *
 * Basically, on some particular access patterns, the read cache misses the
 * fact that the framebuffer has changed, and a glReadPixels returns stale
 * data.
 *
 * The test works by repeatedly rendering a square in different colors,
 * and testing after each run that a number of pixel locations return the
 * right color.
 *
 * @note By the nature of the test, it makes no sense to have a demo mode,
 * so this test is always automatic.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat colors[8][3] = {
	{ 1.0, 1.0, 1.0 },
	{ 1.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
	{ 0.5, 0.0, 0.0 },
	{ 0.0, 0.5, 0.0 },
	{ 0.0, 0.0, 0.5 },
	{ 0.0, 0.0, 0.0 }
};

enum piglit_result piglit_display(void)
{
	int x, y, color, i, comp;
	/* x and y range chosen to cover a wide range of memory;
	 * Actually, only the x coordinate should matter, but who knows... */
	for(y = 0; y < 8; ++y) {
		for(x = 0; x < 32; ++x) {
			for(color = 0; color < 8; ++color) {
				glColor3fv(colors[color]);
				glBegin(GL_QUADS);
				glVertex2f(-1, -1);
				glVertex2f( 1, -1);
				glVertex2f( 1,  1);
				glVertex2f(-1,  1);
				glEnd();

				for(i = 0; i < 2; ++i) {
					GLfloat result[3];
					glReadPixels(x + (i ^ ((color/2)&1))*10, y,
						1, 1, GL_RGB, GL_FLOAT, result);

					for(comp = 0; comp < 3; ++comp) {
						if (fabs(colors[color][comp] - result[comp]) > 0.01) {
							printf("(x,y) = (%i,%i), color=%i, expected: %f %f %f got %f %f %f\n",
								x, y, color,
								colors[color][0], colors[color][1], colors[color][2],
								result[0], result[1], result[2]);
							return PIGLIT_FAIL;
						}
					}
				}
			}
		}
	}

	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	piglit_automatic = GL_TRUE;

	glViewport(0, 0, piglit_width, piglit_height);
}
