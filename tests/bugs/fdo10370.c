/*
 * Test case from fdo bug #10370
 * http://bugs.freedesktop.org/show_bug.cgi?id=10370
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define BITMAP_WIDTH 1
#define BITMAP_HEIGHT 1
#define ALIGN 1
GLfloat read_buf[4 * BITMAP_WIDTH * BITMAP_HEIGHT];
static GLfloat r_map[] = { 0, 1 };
static GLfloat g_map[] = { 0, 0 };
static GLfloat b_map[] = { 1, 0 };
static GLfloat a_map[] = { 1, 1 };
static GLubyte data[] = { 0x8f, 0xff, 0x7f, 0x70 };

static GLuint tex_name;

void
piglit_init(int argc, char **argv)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1.0, -1.0, 0.0);
	glScalef(2.0/piglit_width, 2.0/piglit_height, 1.0);

	glDisable(GL_DITHER);
	glClearColor(1, 1, 1, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

	glPixelMapfv(GL_PIXEL_MAP_I_TO_R, 2, r_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_G, 2, g_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_B, 2, b_map);
	glPixelMapfv(GL_PIXEL_MAP_I_TO_A, 2, a_map);

	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, ALIGN);

	glGenTextures(1, &tex_name);
	glBindTexture(GL_TEXTURE_2D, tex_name);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}



enum piglit_result
piglit_display(void)
{
	int i, j, k, col, pixel;
	GLfloat expected[4];
	float dmax = 0.0;

	memset(read_buf, 0xff, sizeof(read_buf));	//reset

	for (k = 0; k < (sizeof(data)/sizeof(GLubyte)); k ++) {

		glClear(GL_COLOR_BUFFER_BIT);
		glNewList(1, GL_COMPILE_AND_EXECUTE);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, tex_name);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				BITMAP_WIDTH, BITMAP_HEIGHT, 0,
				GL_COLOR_INDEX, GL_BITMAP, &data[k]);

			glBegin(GL_POLYGON);
			glTexCoord2f(0,0); glVertex2f(0, 0);
			glTexCoord2f(1,0); glVertex2f(BITMAP_WIDTH, 0);
			glTexCoord2f(1,1); glVertex2f(BITMAP_WIDTH, BITMAP_HEIGHT);
			glTexCoord2f(0,1); glVertex2f(0, BITMAP_HEIGHT);
			glEnd();
			glDisable(GL_TEXTURE_2D);
		glEndList();
		glFlush();

		glReadPixels(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT,
			GL_RGBA, GL_FLOAT, read_buf);

		printf("data[0x%x], ", data[k]);
		if (data[k] & 0x80) {
			printf("foreground: expected RGBA (%1.1f, %1.1f %1.1f %1.1f)\n",
				r_map[1], g_map[1], b_map[1], a_map[1]);
			expected[0] = r_map[1];
			expected[1] = g_map[1];
			expected[2] = b_map[1];
			expected[3] = a_map[1];
		} else {
			printf("background: expected RGBA (%1.1f, %1.1f %1.1f %1.1f)\n",
				r_map[0], g_map[0], b_map[0], a_map[0]);
			expected[0] = r_map[0];
			expected[1] = g_map[0];
			expected[2] = b_map[0];
			expected[3] = a_map[0];
		}

		printf("First execution, Readback RGBA:\n");
		for (i = 0; i < BITMAP_HEIGHT; i ++) {
			for (j = 0; j < BITMAP_WIDTH; j ++) {
				pixel = j + i*BITMAP_WIDTH;
				printf("pixel[%d, %d]: %1.1f %1.1f %1.1f %1.1f\n", j, i,
					read_buf[pixel*4], read_buf[pixel*4+1],
					read_buf[pixel*4+2], read_buf[pixel*4+3]);

				for(col = 0; col < 4; ++col) {
					float delta = read_buf[pixel*4+col] - expected[col];
					if (delta > dmax) dmax = delta;
					else if (-delta > dmax) dmax = -delta;
				}
			}
		}

		/* 2nd time execution from call list */
		glCallList(1);
		glDeleteLists(1,1);

		memset(read_buf, 0xff, sizeof(read_buf));	//reset
		glReadPixels(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT,
			GL_RGBA, GL_FLOAT, read_buf);

		printf("CallList execution, Readback RGBA:\n");
		for (i = 0; i < BITMAP_HEIGHT; i ++) {
			for (j = 0; j < BITMAP_WIDTH; j ++) {
				pixel = j + i*BITMAP_WIDTH;
				printf("pixel[%d, %d]: %1.1f %1.1f %1.1f %1.1f\n", j, i,
					read_buf[pixel*4], read_buf[pixel*4+1],
					read_buf[pixel*4+2], read_buf[pixel*4+3]);
				for(col = 0; col < 4; ++col) {
					float delta = read_buf[pixel*4+col] - expected[col];
					if (delta > dmax) dmax = delta;
					else if (-delta > dmax) dmax = -delta;
				}
			}
		}
		printf("------------------------------------\n");
	}	//end for(k)

	printf("max delta: %f\n", dmax);

	if (dmax > 0.02)
		return PIGLIT_FAIL;
	else
		return PIGLIT_PASS;
}
