/* Copyright © 2011 Intel Corporation
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

extern PFNGLXGETCURRENTDISPLAYEXTPROC __piglit_glXGetCurrentDisplayEXT;
#define glXGetCurrentDisplayEXT() (*__piglit_glXGetCurrentDisplayEXT)()

extern PFNGLXQUERYCONTEXTINFOEXTPROC __piglit_glXQueryContextInfoEXT;
#define glXQueryContextInfoEXT(dpy, ctx, attrib, value) \
  (*__piglit_glXQueryContextInfoEXT)(dpy, ctx, attrib, value)

extern PFNGLXGETCONTEXTIDEXTPROC __piglit_glXGetContextIDEXT;
#define glXGetContextIDEXT(ctx) (*__piglit_glXGetContextIDEXT)(ctx)

extern PFNGLXIMPORTCONTEXTEXTPROC __piglit_glXImportContextEXT;
#define glXImportContextEXT(dpy, ctx) (*__piglit_glXImportContextEXT)(dpy, ctx)

extern PFNGLXFREECONTEXTEXTPROC __piglit_glXFreeContextEXT;
#define glXFreeContextEXT(dpy, ctx) (*__piglit_glXFreeContextEXT)(dpy, ctx)

extern Display *dpy;
extern XVisualInfo *visinfo;
extern GLXContext directCtx;
extern GLXContextID directID;
extern GLXContext indirectCtx;
extern GLXContextID indirectID;
extern int glx_error_code;
extern int x_error_code;

extern void GLX_EXT_import_context_setup(void);

extern void GLX_EXT_import_context_teardown(void);

extern void GLX_EXT_import_context_setup_for_child(void);

extern void get_context_IDs(void);

enum context_mode {
	direct_rendering = 0,
	indirect_rendering,
	invalid
};

extern bool validate_glx_error_code(int expected_x_error, int expected_glx_error);

extern const char *context_mode_name(enum context_mode mode);

extern bool try_import_context(GLXContextID id, enum context_mode mode);
