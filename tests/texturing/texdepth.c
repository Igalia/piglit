/**
 * @file texdepth.c
 *
 * Basic tests for the following extensions:
 *  - ARB_depth_texture
 *  - ARB_shadow
 *  - ARB_shadow_ambient
 *  - EXT_shadow_funcs
 */

#include "piglit-util-gl.h"

#define ROWS 4
#define COLS 8
static int Width = COLS*32, Height = ROWS*32;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = COLS*32;
	config.window_height = ROWS*32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static int CellWidth, CellHeight;
static int CurrentTest = 0;

static int HaveShadow = 0;
static int HaveShadowAmbient = 0;
static int HaveShadowFuncs = 0;
static GLuint Textures[2]; /* tex0: loaded via TexImage; tex1: loaded via CopyTexImage */


static void CreateRenderedTexture(void)
{
	glViewport(0, 0, COLS*4, ROWS*4);
	glClearColor(0.5, 0.5, 0.0, 0.25);
	glClearDepth(0.25);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glBegin(GL_QUADS);
		glVertex3f(0.25, 0.25, 0.75);
		glVertex3f(0.75, 0.25, 0.75);
		glVertex3f(0.75, 0.75, 0.75);
		glVertex3f(0.25, 0.75, 0.75);
	glEnd();

	glDisable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, Textures[1]);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		0, 0, 4, 4, 0);
}


/**
 * Verify whether the given cell contains the color that is consistent
 * with a depth texture result of \p value in the given depth texture mode.
 */
static int probe_cell_depth_mode(int cellx, int celly, int depth_texture_mode, float value)
{
	float expected[4] = { 1.0, 1.0, 1.0, 1.0 };
        int res;

	switch(depth_texture_mode) {
	case GL_INTENSITY:
		expected[3] = value;
		/* fall through */
	case GL_LUMINANCE:
		expected[0] = value;
		expected[1] = value;
		expected[2] = value;
		break;
	case GL_ALPHA:
		/* Note: alpha values v are translated into RGBA (0,0,0,v),
		 * but texture environments ignore the RGB component for alpha
		 * textures, so in the end we're back to RGB being white */
		expected[3] = value;
		break;
	}

	res = piglit_probe_pixel_rgba(
		cellx*CellWidth + (CellWidth/2), celly*CellHeight + (CellHeight/2),
		expected);

	return res;
}


/**
 * Render the depth textures directly, without any texture comparisons.
 */
static int test_RenderTextures(int param)
{
	int succ = 1;

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, param);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2f(0,0);
		glTexCoord2f(1,0);
		glVertex2f(4,0);
		glTexCoord2f(1,1);
		glVertex2f(4,4);
		glTexCoord2f(0,1);
		glVertex2f(0,4);
	glEnd();
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);

	glBindTexture(GL_TEXTURE_2D, Textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, param);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);
		glVertex2f(4,0);
		glTexCoord2f(1,0);
		glVertex2f(8,0);
		glTexCoord2f(1,1);
		glVertex2f(8,4);
		glTexCoord2f(0,1);
		glVertex2f(4,4);
	glEnd();
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);

	glDisable(GL_TEXTURE_2D);

	/* Note: Report only the first failed probe */
	succ = succ && probe_cell_depth_mode(0, 0, param, 0.0);
	succ = succ && probe_cell_depth_mode(2, 0, param, 0.25);
	succ = succ && probe_cell_depth_mode(0, 2, param, 0.5);
	succ = succ && probe_cell_depth_mode(2, 2, param, 1.0);

	succ = succ && probe_cell_depth_mode(4, 0, param, 0.25);
	succ = succ && probe_cell_depth_mode(4, 1, param, 0.25);
	succ = succ && probe_cell_depth_mode(4, 2, param, 0.25);
	succ = succ && probe_cell_depth_mode(4, 3, param, 0.25);
	succ = succ && probe_cell_depth_mode(5, 0, param, 0.25);
	succ = succ && probe_cell_depth_mode(5, 1, param, 0.75);
	succ = succ && probe_cell_depth_mode(5, 2, param, 0.75);
	succ = succ && probe_cell_depth_mode(5, 3, param, 0.25);
	succ = succ && probe_cell_depth_mode(6, 0, param, 0.25);
	succ = succ && probe_cell_depth_mode(6, 1, param, 0.75);
	succ = succ && probe_cell_depth_mode(6, 2, param, 0.75);
	succ = succ && probe_cell_depth_mode(6, 3, param, 0.25);
	succ = succ && probe_cell_depth_mode(7, 0, param, 0.25);
	succ = succ && probe_cell_depth_mode(7, 1, param, 0.25);
	succ = succ && probe_cell_depth_mode(7, 2, param, 0.25);
	succ = succ && probe_cell_depth_mode(7, 3, param, 0.25);

	return succ;
}


static float texture_compare(int comparefunc, float r, float texture, float ambient)
{
	int test = 0;
	switch(comparefunc) {
	case GL_NEVER: test = 0; break;
	case GL_LESS: test = r < texture; break;
	case GL_LEQUAL: test = r <= texture; break;
	case GL_EQUAL: test = r == texture; break;
	case GL_NOTEQUAL: test = r != texture; break;
	case GL_GEQUAL: test = r >= texture; break;
	case GL_GREATER: test = r > texture; break;
	case GL_ALWAYS: test = 1; break;
	default: assert(0);
	}
	return test ? 1.0 : ambient;
}

static int test_worker(int comparefunc, float ambient, float w)
{
	int succ = 1;

	if (!HaveShadow)
		return 1;
	if (!HaveShadowFuncs && (comparefunc != GL_LEQUAL && comparefunc != GL_GEQUAL))
		return 1;
	if (!HaveShadowAmbient && ambient > 0.0)
		return 1;

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, Textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, comparefunc);
	if (ambient > 0.0)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, ambient);
	glBegin(GL_QUADS);
		glTexCoord4f(w*0,w*0,w*0.6,w);
		glVertex2f(0,0);
		glTexCoord4f(w*1,w*0,w*0.6,w);
		glVertex2f(4,0);
		glTexCoord4f(w*1,w*1,w*0.6,w);
		glVertex2f(4,4);
		glTexCoord4f(w*0,w*1,w*0.6,w);
		glVertex2f(0,4);
	glEnd();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	if (ambient > 0.0)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.0);

	glBindTexture(GL_TEXTURE_2D, Textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, comparefunc);
	if (ambient > 0.0)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, ambient);
	glBegin(GL_QUADS);
		glTexCoord4f(w*0.0,w*0.0,w*0.2,w);
		glVertex2f(4,0);
		glTexCoord4f(w*0.5,w*0.0,w*0.2,w);
		glVertex2f(6,0);
		glTexCoord4f(w*0.5,w*0.5,w*0.2,w);
		glVertex2f(6,2);
		glTexCoord4f(w*0.0,w*0.5,w*0.2,w);
		glVertex2f(4,2);

		glTexCoord4f(w*0.5,w*0.0,w*0.5,w);
		glVertex2f(6,0);
		glTexCoord4f(w*1.0,w*0.0,w*0.5,w);
		glVertex2f(8,0);
		glTexCoord4f(w*1.0,w*0.5,w*0.5,w);
		glVertex2f(8,2);
		glTexCoord4f(w*0.5,w*0.5,w*0.5,w);
		glVertex2f(6,2);

		glTexCoord4f(w*0.0,w*0.5,w*0.8,w);
		glVertex2f(4,2);
		glTexCoord4f(w*0.5,w*0.5,w*0.8,w);
		glVertex2f(6,2);
		glTexCoord4f(w*0.5,w*1.0,w*0.8,w);
		glVertex2f(6,4);
		glTexCoord4f(w*0.0,w*1.0,w*0.8,w);
		glVertex2f(4,4);
	glEnd();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	if (ambient > 0.0)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.0);

	glDisable(GL_TEXTURE_2D);

	succ = succ && probe_cell_depth_mode(0, 0, GL_LUMINANCE, texture_compare(comparefunc, 0.6, 0.0, ambient));
	succ = succ && probe_cell_depth_mode(2, 0, GL_LUMINANCE, texture_compare(comparefunc, 0.6, 0.25, ambient));
	succ = succ && probe_cell_depth_mode(0, 2, GL_LUMINANCE, texture_compare(comparefunc, 0.6, 0.5, ambient));
	succ = succ && probe_cell_depth_mode(2, 2, GL_LUMINANCE, texture_compare(comparefunc, 0.6, 1.0, ambient));

	succ = succ && probe_cell_depth_mode(4, 0, GL_LUMINANCE, texture_compare(comparefunc, 0.2, 0.25, ambient));
	succ = succ && probe_cell_depth_mode(5, 1, GL_LUMINANCE, texture_compare(comparefunc, 0.2, 0.75, ambient));

	succ = succ && probe_cell_depth_mode(6, 0, GL_LUMINANCE, texture_compare(comparefunc, 0.5, 0.25, ambient));
	succ = succ && probe_cell_depth_mode(6, 1, GL_LUMINANCE, texture_compare(comparefunc, 0.5, 0.75, ambient));

	succ = succ && probe_cell_depth_mode(4, 3, GL_LUMINANCE, texture_compare(comparefunc, 0.8, 0.25, ambient));
	succ = succ && probe_cell_depth_mode(5, 2, GL_LUMINANCE, texture_compare(comparefunc, 0.8, 0.75, ambient));

	return succ;
}

static int test_BasicShadow(int comparefunc)
{
	return test_worker(comparefunc, 0.0, 1.0);
}

static int test_AmbientShadow(int comparefunc)
{
	return test_worker(comparefunc, 0.4, 1.0);
}

static int test_Homogenous(int comparefunc)
{
	int succ = test_worker(comparefunc, 0.0, 3.3);
	succ = succ && test_worker(comparefunc, 0.4, 3.3);
	return succ;
}

struct test_step {
	int (*func)(int);
	int param;
	const char* name;
};

static struct test_step Tests[] = {
	{ test_RenderTextures, GL_LUMINANCE, "Render textures GL_LUMINANCE (no shadow functionality)" },
	{ test_RenderTextures, GL_INTENSITY, "Render textures GL_INTENSITY (no shadow functionality)" },
	{ test_RenderTextures, GL_ALPHA, "Render textures GL_ALPHA (no shadow functionality)" },
	{ test_BasicShadow, GL_NEVER, "EXT_shadow_func: GL_NEVER" },
	{ test_BasicShadow, GL_LESS, "EXT_shadow_func: GL_LESS" },
	{ test_BasicShadow, GL_LEQUAL, "ARB_shadow: GL_LEQUAL" },
	/* don't test GL_EQUAL and GL_NOTEQUAL: they're bound to be unreliable due to precision problems */
	{ test_BasicShadow, GL_GEQUAL, "ARB_shadow: GL_GEQUAL" },
	{ test_BasicShadow, GL_GREATER, "EXT_shadow_func: GL_GREATER" },
	{ test_BasicShadow, GL_ALWAYS, "EXT_shadow_func: GL_ALWAYS" },
	{ test_AmbientShadow, GL_NEVER, "Ambient + EXT_shadow_func: GL_NEVER" },
	{ test_AmbientShadow, GL_LESS, "Ambient + EXT_shadow_func: GL_LESS" },
	{ test_AmbientShadow, GL_LEQUAL, "Ambient + ARB_shadow: GL_LEQUAL" },
	/* don't test GL_EQUAL and GL_NOTEQUAL: they're bound to be unreliable due to precision problems */
	{ test_AmbientShadow, GL_GEQUAL, "Ambient + ARB_shadow: GL_GEQUAL" },
	{ test_AmbientShadow, GL_GREATER, "Ambient + EXT_shadow_func: GL_GREATER" },
	{ test_AmbientShadow, GL_ALWAYS, "Ambient + EXT_shadow_func: GL_ALWAYS" },
	{ test_Homogenous, GL_NEVER, "homogenous: GL_NEVER" },
	{ test_Homogenous, GL_LESS, "homogenous: GL_LESS" },
	{ test_Homogenous, GL_LEQUAL, "homogenous: GL_LEQUAL" },
	/* don't test GL_EQUAL and GL_NOTEQUAL: they're bound to be unreliable due to precision problems */
	{ test_Homogenous, GL_GEQUAL, "homogenous: GL_GEQUAL" },
	{ test_Homogenous, GL_GREATER, "homogenous: GL_GREATER" },
	{ test_Homogenous, GL_ALWAYS, "homogenous: GL_ALWAYS" },
};
#define NumTests (ARRAY_SIZE(Tests))

enum piglit_result
piglit_display(void)
{
	CellWidth = piglit_width / COLS;
	CellHeight = piglit_height / ROWS;
	Width = CellWidth*COLS;
	Height = CellHeight*ROWS;
	piglit_gen_ortho_projection(0.0, COLS, 0.0, ROWS, 0.0, -1.0, GL_FALSE);

	glReadBuffer(GL_BACK);
	CreateRenderedTexture();
	glViewport(0, 0, Width, Height);

	if (piglit_automatic) {
		int succ = 1;
		int i;

		for(i = 0; i < NumTests; ++i) {
			glClearColor(0.5, 0.5, 0.0, 0.6);
			glClearDepth(0.25);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			succ = Tests[i].func(Tests[i].param);
			if (!succ) {
				printf("Test failed: '%s'\nSee above for details.\n\n", Tests[i].name);
				break;
			}
		}

		return succ ? PIGLIT_PASS : PIGLIT_FAIL;
	}

	glClearColor(0.5, 0.5, 0.0, 0.6);
	glClearDepth(0.25);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Tests[CurrentTest].func(Tests[CurrentTest].param);

	piglit_present_results();

	return PIGLIT_PASS;
}

static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
	case 't':
		CurrentTest++;
		if (CurrentTest >= NumTests)
			CurrentTest = 0;
		printf("Now showing test: %s\n", Tests[CurrentTest].name);
		break;
	case 27:
		exit(0);
		break;
	}
	piglit_post_redisplay();
}

void
piglit_init(int argc, char **argv)
{
	GLfloat texbuf[4];

	if (!piglit_automatic) {
		piglit_set_keyboard_func(Key);
		printf("Press 't' to cycle through test images\n");
	}

	piglit_require_extension("GL_ARB_depth_texture");
	HaveShadow = piglit_is_extension_supported("GL_ARB_shadow");
	if (!HaveShadow)
		printf("GL_ARB_shadow not supported.\n");
	HaveShadowAmbient = piglit_is_extension_supported("GL_ARB_shadow_ambient");
	if (!HaveShadowAmbient)
		printf("GL_ARB_shadow_ambient not supported.\n");
	HaveShadowFuncs = piglit_is_extension_supported("GL_EXT_shadow_funcs");
	if (!HaveShadowFuncs)
		printf("GL_EXT_shadow_funcs not supported.\n");

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glGenTextures(2, Textures);
	glBindTexture(GL_TEXTURE_2D, Textures[0]);

	texbuf[0] = 0.0;
	texbuf[1] = 0.25;
	texbuf[2] = 0.5;
	texbuf[3] = 1.0;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2, 2, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, texbuf);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, Textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4, 4, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
