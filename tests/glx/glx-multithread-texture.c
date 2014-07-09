/*
 * Copyright 2013 Google Inc.
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

/** @file glx-multithread-texture.c
 *
 * Test loading texture data from one thread and context while drawing with
 * those textures from another thread and shared context.  The threads are
 * synchronized so they do not attempt to use the same texture at the same time.
 */

#include <unistd.h>

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "pthread.h"

int piglit_width = 50, piglit_height = 50;
static int tex_width = 512, tex_height = 512;
static unsigned num_test = 300;
static bool quit = false;

static Display *dpy;
static Window draw_win;
static GLXPixmap load_win;
static pthread_mutex_t mutex;
static XVisualInfo *visinfo;
static GLXContext draw_ctx, load_ctx;

/*
 * list of textures
 *
 * user == DRAW means draw thread may draw with this texture
 * user == LOAD means load thread may load data into this texture
 * user == NONE means texture not in use and may be claimed by either thread
 *
 * Minimum of three items needed in this array for the code to work.
 */
static struct texture {
	GLuint id;
	int color;
	enum user { DRAW, LOAD, NONE } user;
} texture[5];

/*
 * If texture at *pos is not in use claim it for 'user' and increment *pos.
 * Return texture at *pos.
 */
static struct texture *
advance(int *pos, enum user user)
{
	int cur = *pos % ARRAY_SIZE(texture);
	int next = (cur + 1) % ARRAY_SIZE(texture);

	pthread_mutex_lock(&mutex);
	assert(texture[cur].user == user);
	if (texture[next].user == NONE) {
		texture[cur].user = NONE;
		cur = next;
		*pos += 1;
		texture[cur].user = user;
	}
	pthread_mutex_unlock(&mutex);

	/* helps avoid starvation */
	usleep(1);

	return texture + cur;
}

/*
 * Texture writing thread: loads data into successive textures, taking note
 * of what color was used so it can be checked later.
 * Return NULL on failure, else non-NULL.
 */
static void *
load_func(void *arg)
{
	int count = 1;
	struct texture *tex = texture + count;
	unsigned int tex_bytes = tex_width * tex_height * 4;
	unsigned char *tex_data = malloc(tex_bytes);
	int ret;

	ret = glXMakeCurrent(dpy, load_win, load_ctx);
	assert(ret);

	glEnable(GL_TEXTURE_2D);

	while (!quit && count <= num_test) {
		int color = count & 0xff;

		assert(tex->user == LOAD);
		if (tex->color != color) {
			memset(tex_data, color, tex_bytes);
			tex->color = color;
		}
		glBindTexture(GL_TEXTURE_2D, tex->id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);

		tex = advance(&count, LOAD);
	}

	glFinish();
	free(tex_data);

	if (count <= num_test) {
		quit = true;
		return NULL;
	}

	return "";
}

/*
 * Texture using thread: draws with successive textures and checks that the
 * correct color is drawn.  Return NULL on failure, else non-NULL.
 */
static void *
draw_func(void *arg)
{
	int count = 0;
	int ret;

	ret = glXMakeCurrent(dpy, draw_win, draw_ctx);
	assert(ret);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	while (!quit && count < num_test) {
		struct texture *tex = advance(&count, DRAW);
		float expect[] = {
			tex->color / 255.,
			tex->color / 255.,
			tex->color / 255.,
		};

		assert(tex->user == DRAW);

		glBindTexture(GL_TEXTURE_2D, tex->id);
		piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
				     0, 0, 1, 1);
		glXSwapBuffers(dpy, draw_win);

		/* first texture not filled so don't check it */
		if (count > 0 &&
			!piglit_probe_rect_rgb(0, 0, piglit_width,
					       piglit_height, expect)) {
			break;
		}
	}

	if (count < num_test) {
		quit = true;
		return NULL;
	}

	return "";
}

enum piglit_result
draw(Display *dpy)
{
	pthread_t draw_thread, load_thread;
	void *draw_ret, *load_ret;
	int ret, i;
	GLXContext my_ctx;

	my_ctx = piglit_get_glx_context_share(dpy, visinfo, NULL);
	draw_ctx = piglit_get_glx_context_share(dpy, visinfo, my_ctx);
	load_ctx = piglit_get_glx_context_share(dpy, visinfo, my_ctx);

	ret = glXMakeCurrent(dpy, draw_win, my_ctx);
	assert(ret);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	glEnable(GL_TEXTURE_2D);

	for (i = 0; i < ARRAY_SIZE(texture); ++i) {
		glGenTextures(1, &texture[i].id);
		texture[i].color = -1;
		texture[i].user = NONE;
		glBindTexture(GL_TEXTURE_2D, texture[i].id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	texture[0].user = DRAW;
	texture[1].user = LOAD;

	pthread_mutex_init(&mutex, NULL);

	pthread_create(&draw_thread, NULL, draw_func, NULL);
	pthread_create(&load_thread, NULL, load_func, NULL);

	ret = pthread_join(draw_thread, &draw_ret);
	assert(ret == 0);
	ret = pthread_join(load_thread, &load_ret);
	assert(ret == 0);

	pthread_mutex_destroy(&mutex);

	glXDestroyContext(dpy, load_ctx);
	glXDestroyContext(dpy, draw_ctx);
	glXDestroyContext(dpy, my_ctx);

	return draw_ret && load_ret ? PIGLIT_PASS : PIGLIT_FAIL;
}

int
main(int argc, char **argv)
{
	int i;
	Pixmap pixmap;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	XInitThreads();
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	visinfo = piglit_get_glx_visual(dpy);
	draw_win = piglit_get_glx_window(dpy, visinfo);
	pixmap = XCreatePixmap(dpy, DefaultRootWindow(dpy),
		piglit_width, piglit_height, visinfo->depth);
	load_win = glXCreateGLXPixmap(dpy, visinfo, pixmap);

	XMapWindow(dpy, draw_win);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
