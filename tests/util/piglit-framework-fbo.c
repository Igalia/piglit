/*
 * Copyright © 2009 Intel Corporation
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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "piglit-util.h"

#ifdef USE_GLX
#include "piglit-glx-util.h"
#endif

#ifdef USE_GLX
Display *piglit_glx_dpy;
Window piglit_glx_window;
XVisualInfo *piglit_glx_visinfo;
GLXContext piglit_glx_context;
#endif

#ifdef USE_GLX
static void
piglit_framework_fbo_glx_init()
{
	piglit_glx_dpy = piglit_get_glx_display();

	/* Unfortunately in GLX we need a drawable to bind our context
	 * to.  Make an unmapped window.
	 */
	piglit_glx_visinfo = piglit_get_glx_visual(piglit_glx_dpy);

	piglit_glx_context = piglit_get_glx_context(piglit_glx_dpy,
						    piglit_glx_visinfo);

	piglit_glx_window = piglit_get_glx_window_unmapped(piglit_glx_dpy,
							   piglit_glx_visinfo);

	glXMakeCurrent(piglit_glx_dpy, piglit_glx_window, piglit_glx_context);
}
#endif

static void
piglit_framework_fbo_glx_destroy()
{
#ifdef USE_GLX
	glXMakeCurrent(piglit_glx_dpy, None, None);
	glXDestroyContext(piglit_glx_dpy, piglit_glx_context);
	XFree(piglit_glx_visinfo);
	XCloseDisplay(piglit_glx_dpy);
#endif
}

static void
piglit_framework_fbo_destroy()
{
#ifdef USE_OPENGL
	glDeleteFramebuffers(1, &piglit_winsys_fbo);
#endif
	piglit_winsys_fbo = 0;
	piglit_framework_fbo_glx_destroy();
}

static bool
piglit_framework_fbo_init()
{
#ifdef USE_GLX
	GLuint tex, depth = 0;
	GLenum status;

	piglit_framework_fbo_glx_init();

#ifdef USE_OPENGL
	glewInit();

	if (piglit_get_gl_version() < 20)
		return false;
#endif

	glGenFramebuffers(1, &piglit_winsys_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D,
			       tex,
			       0);

	if (piglit_window_mode & (GLUT_DEPTH | GLUT_STENCIL)) {
		GLenum depth_stencil;

#ifdef USE_OPENGL
		depth_stencil = GL_DEPTH_STENCIL;
#else
		depth_stencil = GL_DEPTH_STENCIL_OES;
#endif

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, depth_stencil,
			     piglit_width, piglit_height, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_DEPTH_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_STENCIL_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"-fbo resulted in incomplete FBO, falling back\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteTextures(1, &depth);
		glDeleteTextures(1, &tex);

		piglit_framework_fbo_destroy();

		return false;
	}

	return true;
#else /* USE_GLX */
	return false;
#endif /* USE_GLX */
}
