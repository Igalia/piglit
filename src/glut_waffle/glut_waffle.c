/*
 * Copyright 2012 Intel Corporation
 * Copyright 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "priv/common.h"

void
glutInitAPIMask(int mask)
{
	switch (mask) {
		case GLUT_OPENGL_BIT:
			_glut->waffle_context_api = WAFFLE_CONTEXT_OPENGL;
			break;
		case GLUT_OPENGL_ES1_BIT:
			_glut->waffle_context_api = WAFFLE_CONTEXT_OPENGL_ES1;
			break;
		case GLUT_OPENGL_ES2_BIT:
			_glut->waffle_context_api = WAFFLE_CONTEXT_OPENGL_ES2;
			break;
		default:
			glutFatal("api_mask has bad value %#x", mask);
			break;
	}
}

void
glutInit(int *argcp, char **argv)
{
	const char *piglit_platform;
	const char *display_name = NULL;
	bool ok = true;
	int i;

	int32_t waffle_init_attrib_list[] = {
		WAFFLE_PLATFORM,        0x31415925,
		0,
	};

	for (i = 1; i < *argcp; i++) {
		if (strcmp(argv[i], "-display") == 0)
			display_name = argv[++i];
		else if (strcmp(argv[i], "-info") == 0) {
			printf("waffle_glut: ignoring -info\n");
		}
	}

	_glut->waffle_context_api = WAFFLE_CONTEXT_OPENGL;

	piglit_platform = getenv("PIGLIT_PLATFORM");
	if (piglit_platform == NULL) {
		_glut->waffle_platform = WAFFLE_PLATFORM_GLX;
	} else if (!strcmp(piglit_platform, "glx")) {
		_glut->waffle_platform = WAFFLE_PLATFORM_GLX;
	} else if (!strcmp(piglit_platform, "x11_egl")) {
		_glut->waffle_platform = WAFFLE_PLATFORM_X11_EGL;
	} else if (!strcmp(piglit_platform, "wayland")) {
		_glut->waffle_platform = WAFFLE_PLATFORM_WAYLAND;
	} else {
		glutFatal("environment var PIGLIT_PLATFORM has bad "
			  "value \"%s\"", piglit_platform);
	}

	waffle_init_attrib_list[1] = _glut->waffle_platform;
	ok = waffle_init(waffle_init_attrib_list);
	if (!ok)
		glutFatal("waffle_init() failed");

	_glut->display = waffle_display_connect(display_name);
	if (!_glut->display)
		glutFatal("waffle_display_connect() failed");
}

void
glutInitDisplayMode(unsigned int mode)
{
	_glut->display_mode = mode;
}

void
glutInitWindowPosition(int x, int y)
{
	// empty
}

void
glutInitWindowSize(int width, int height)
{
	_glut->window_width = width;
	_glut->window_height = height;
}

static struct waffle_config*
glutChooseConfig(void)
{
	struct waffle_config *config = NULL;
	int32_t attrib_list[64];
	int i = 0;

	#define ADD_ATTR(name, value) \
		do { \
			attrib_list[i++] = name; \
			attrib_list[i++] = value; \
		} while (0)

	ADD_ATTR(WAFFLE_CONTEXT_API, _glut->waffle_context_api);

	/* It is impossible to not request RGBA because GLUT_RGB and
	 * GLUT_RGBA are both 0. That is, (display_mode & (GLUT_RGB
	 * | GLUT_RGBA)) is unconditonally true.
	 */
	ADD_ATTR(WAFFLE_RED_SIZE,   1);
	ADD_ATTR(WAFFLE_GREEN_SIZE, 1);
	ADD_ATTR(WAFFLE_BLUE_SIZE,  1);
	ADD_ATTR(WAFFLE_ALPHA_SIZE, 1);

	if (_glut->display_mode & GLUT_DEPTH) {
		ADD_ATTR(WAFFLE_DEPTH_SIZE, 1);
	}

	if (_glut->display_mode & GLUT_STENCIL) {
		ADD_ATTR(WAFFLE_STENCIL_SIZE, 1);
	}

	if (!(_glut->display_mode & GLUT_DOUBLE)) {
		ADD_ATTR(WAFFLE_DOUBLE_BUFFERED, false);
	}

	if (_glut->display_mode & GLUT_ACCUM) {
		ADD_ATTR(WAFFLE_ACCUM_BUFFER, true);
	}

	attrib_list[i++] = WAFFLE_NONE;

	config = waffle_config_choose(_glut->display, attrib_list);
	if (!config)
		glutFatal("waffle_config_choose() failed");
	return config;
}

void
glutPostRedisplay(void)
{
	_glut->redisplay = 1;
}

static void
_glutDefaultKeyboard(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}

int
glutCreateWindow(const char *title)
{
	bool ok = true;
	struct waffle_config *config = NULL;

	if (_glut->window)
		glutFatal("cannot create window; one already exists");

	config = glutChooseConfig();

	_glut->context = waffle_context_create(config, NULL);
	if (!_glut->context)
		glutFatal("waffle_context_create() failed");

	_glut->window = calloc(1, sizeof(*_glut->window));
	if (!_glut->window)
		glutFatal("out of memory");

	_glut->window->waffle = waffle_window_create(config,
	                                             _glut->window_width,
	                                             _glut->window_height);
	if (!_glut->window->waffle)
		glutFatal("waffle_window_create() failed");

	ok = waffle_make_current(_glut->display, _glut->window->waffle,
			_glut->context);
	if (!ok)
		glutFatal("waffle_make_current() failed");

	_glut->window->id = ++_glut->window_id_pool;
	_glut->window->keyboard_cb = _glutDefaultKeyboard;

	return _glut->window->id;
}

void
glutDestroyWindow(int win)
{
	bool ok = true;

	if (!_glut->window || _glut->window->id != win)
		glutFatal("bad window id");

	ok = waffle_window_destroy(_glut->window->waffle);
	if (!ok)
		glutFatal("waffle_window_destroy() failed");

	free(_glut->window);
	_glut->window = NULL;
}

void
glutShowWindow(int win)
{
	bool ok = true;

	if (!_glut->window || _glut->window->id != win)
		glutFatal("bad window id");

	ok = waffle_window_show(_glut->window->waffle);
	if (!ok)
		glutFatal("waffle_window_show() failed");
}

void
glutDisplayFunc(GLUT_EGLdisplayCB func)
{
	_glut->window->display_cb = func;
}

void
glutReshapeFunc(GLUT_EGLreshapeCB func)
{
	_glut->window->reshape_cb = func;
}

void
glutKeyboardFunc(GLUT_EGLkeyboardCB func)
{
	_glut->window->keyboard_cb = func;
}

void
glutMainLoop(void)
{
	bool ok = true;

	if (!_glut->window)
		glutFatal("no window is created");

	ok = waffle_window_show(_glut->window->waffle);
	if (!ok)
		glutFatal("waffle_window_show() failed");

	if (_glut->window->reshape_cb)
		_glut->window->reshape_cb(_glut->window_width,
		                          _glut->window_height);

	if (_glut->window->display_cb)
		_glut->window->display_cb();

	// FIXME: Tests run without -auto require basic input.

	// Workaround for input:
	// Since glut_waffle doesn't handle input yet, it sleeps in order to
	// give the user a chance to see the test output. If the user wishes
	// the test to sleep for a shorter or longer time, he can use Ctrl-C
	// or Ctrl-Z.
	sleep(20);
}

void
glutSwapBuffers(void)
{
	bool ok = waffle_window_swap_buffers(_glut->window->waffle);
	if (!ok)
		glutFatal("waffle_window_swap_buffers() failed");
}
