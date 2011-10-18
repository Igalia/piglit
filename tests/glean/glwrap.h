// BEGIN_COPYRIGHT
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT




// Microsoft's version of gl.h invokes macros that are defined in
// windows.h.  To avoid a conditional #include <windows.h> in
// every file, we wrap gl.h with the proper conditions here, and
// have our source files #include "glwrap.h" instead.

// As a bonus we ensure that all declarations for GLU are included,
// and on X11-based systems, we cover X11 and GLX as well.  This
// should cover nearly everything needed by a typical glean test.

// It's unfortunate that both Windows and Xlib are so casual about
// polluting the global namespace.  The problem isn't easily resolved,
// even with the use of C++ namespace directives, because (a) macros
// in the include files refer to unqualified global variables, and (b)
// preprocessor macros themselves aren't affected by namespaces.


#ifndef __glwrap_h__
#define __glwrap_h__

#if defined(__WIN__)
#  include <windows.h>
#  include <GL/gl.h>
#  include <GL/glu.h>
#  if !defined(GLAPIENTRY)
#      define GLAPIENTRY __stdcall
#  endif
#  if !defined(GLCALLBACK)
#      define GLCALLBACK __stdcall
#  endif
#  include <GL/glext.h>
#elif defined(__X11__)
#  include <GL/glx.h>
   // glx.h covers Xlib.h and gl.h, among others
#  include <GL/glu.h>
#  if !defined(GLAPIENTRY)
#      define GLAPIENTRY
#  endif
#  if !defined(GLCALLBACK)
#      define GLCALLBACK
#  endif
#  include <GL/glext.h>
#elif defined(__AGL__)
#  include <Carbon/Carbon.h>
#  include <OpenGL/glu.h>
#  include <OpenGL/glext.h>
#  include <AGL/agl.h>
#  include <AGL/aglRenderers.h>
#  if !defined(APIENTRY)
#      define APIENTRY
#  endif
#  if !defined(GLAPIENTRY)
#      define GLAPIENTRY
#  endif
#  if !defined(GLCALLBACK)
#      define GLCALLBACK
#  endif
#  if !defined(sinf)
#      define sinf sin
#      define cosf cos
#      define sqrtf sqrt
#  endif
#else
#  error "Improper window system configuration; must be __WIN__ or __X11__."
#endif

#ifndef GL_COMBINE_EXT
	#ifdef GL_COMBINE_ARB
		#define GL_COMBINE_EXT 			GL_COMBINE_ARB                    
		#define GL_COMBINE_RGB_EXT 		GL_COMBINE_RGB_ARB
		#define GL_COMBINE_ALPHA_EXT 	GL_COMBINE_ALPHA_ARB
		#define GL_RGB_SCALE_EXT 		GL_RGB_SCALE_ARB
		#define GL_ADD_SIGNED_EXT 		GL_ADD_SIGNED_ARB
		#define GL_INTERPOLATE_EXT		GL_INTERPOLATE_ARB
		#define GL_CONSTANT_EXT			GL_CONSTANT_ARB
		#define GL_PRIMARY_COLOR_EXT	GL_PRIMARY_COLOR_ARB
		#define GL_PREVIOUS_EXT			GL_PREVIOUS_ARB
		#define GL_SUBTRACT_EXT			GL_SUBTRACT_ARB
		#define GL_SOURCE0_RGB_EXT		GL_SOURCE0_RGB_ARB
		#define GL_SOURCE1_RGB_EXT		GL_SOURCE1_RGB_ARB
		#define GL_SOURCE2_RGB_EXT		GL_SOURCE2_RGB_ARB
		#define GL_SOURCE0_ALPHA_EXT	GL_SOURCE0_ALPHA_ARB
		#define GL_SOURCE1_ALPHA_EXT	GL_SOURCE1_ALPHA_ARB
		#define GL_SOURCE2_ALPHA_EXT	GL_SOURCE2_ALPHA_ARB
		#define GL_OPERAND0_RGB_EXT     GL_OPERAND0_RGB_ARB
		#define GL_OPERAND1_RGB_EXT		GL_OPERAND1_RGB_ARB
		#define GL_OPERAND2_RGB_EXT		GL_OPERAND2_RGB_ARB
		#define GL_OPERAND0_ALPHA_EXT	GL_OPERAND0_ALPHA_ARB
		#define GL_OPERAND1_ALPHA_EXT   GL_OPERAND1_ALPHA_ARB
		#define GL_OPERAND2_ALPHA_EXT	GL_OPERAND2_ALPHA_ARB
	#endif
#endif


#ifndef GL_EXT_texture_swizzle
#define GL_TEXTURE_SWIZZLE_R_EXT          0x8E42
#define GL_TEXTURE_SWIZZLE_G_EXT          0x8E43
#define GL_TEXTURE_SWIZZLE_B_EXT          0x8E44
#define GL_TEXTURE_SWIZZLE_A_EXT          0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA_EXT       0x8E46
#endif


#ifndef GL_EXT_texture_sRGB
#define GL_SRGB_EXT                       0x8C40
#define GL_SRGB8_EXT                      0x8C41
#define GL_SRGB_ALPHA_EXT                 0x8C42
#define GL_SRGB8_ALPHA8_EXT               0x8C43
#define GL_SLUMINANCE_ALPHA_EXT           0x8C44
#define GL_SLUMINANCE8_ALPHA8_EXT         0x8C45
#define GL_SLUMINANCE_EXT                 0x8C46
#define GL_SLUMINANCE8_EXT                0x8C47
#define GL_COMPRESSED_SRGB_EXT            0x8C48
#define GL_COMPRESSED_SRGB_ALPHA_EXT      0x8C49
#define GL_COMPRESSED_SLUMINANCE_EXT      0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT 0x8C4B
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#endif


#ifndef GL_NV_texture_env_combine4
#define GL_COMBINE4_NV                    0x8503
#define GL_SOURCE3_RGB_NV                 0x8583
#define GL_SOURCE3_ALPHA_NV               0x858B
#define GL_OPERAND3_RGB_NV                0x8593
#define GL_OPERAND3_ALPHA_NV              0x859B
#endif


#ifndef GL_NV_texture_env_combine4
#define GL_COMBINE4_NV                    0x8503
#define GL_SOURCE3_RGB_NV                 0x8583
#define GL_SOURCE3_ALPHA_NV               0x858B
#define GL_OPERAND3_RGB_NV                0x8593
#define GL_OPERAND3_ALPHA_NV              0x859B
#endif


#ifndef GL_EXT_provoking_vertex
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION_EXT    0x8E4D
#define GL_LAST_VERTEX_CONVENTION_EXT     0x8E4E
#define GL_PROVOKING_VERTEX_EXT           0x8E4F
#endif


#ifndef GL_ARB_provoking_vertex
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION    0x8E4D
#define GL_LAST_VERTEX_CONVENTION     0x8E4E
#define GL_PROVOKING_VERTEX           0x8E4F
#endif


#ifndef GL_ARB_map_buffer_range
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
#endif


#ifdef __APPLE__
typedef unsigned short GLhalfARB;
#endif


// Windows has a convention for typedef'ing pointers to OpenGL functions
// which encapsulates some of the oddities of Win32 calling conventions.
// Identical conventions are being established for Linux, but they are
// not yet in place, and are not necessarily supported in other UNIX
// variants.  Therefore we need a similar mechanism that accomplishes
// the same goal.  We'll use the standard typedef names, but put them
// in the GLEAN namespace; that should preserve as much source-code
// compatibility as possible elsewhere in glean.

namespace GLEAN {
typedef void (GLAPIENTRY * PFNGLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY * PFNGLPOLYGONOFFSETEXTPROC) (GLfloat factor, GLfloat bias);
typedef void (GLAPIENTRY * PFNGLTEXIMAGE3DEXTPROC) (GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE3DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLGETTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLfloat *weights);
typedef void (GLAPIENTRY * PFNGLTEXFILTERFUNCSGISPROC) (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights);
typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE1DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLCOPYTEXIMAGE1DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
typedef void (GLAPIENTRY * PFNGLCOPYTEXIMAGE2DEXTPROC) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE2DEXTPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLGETHISTOGRAMEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (GLAPIENTRY * PFNGLGETHISTOGRAMPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETHISTOGRAMPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETMINMAXEXTPROC) (GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values);
typedef void (GLAPIENTRY * PFNGLGETMINMAXPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETMINMAXPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLHISTOGRAMEXTPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (GLAPIENTRY * PFNGLMINMAXEXTPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (GLAPIENTRY * PFNGLRESETHISTOGRAMEXTPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLRESETMINMAXEXTPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERFEXTPROC) (GLenum target, GLenum pname, GLfloat params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERIEXTPROC) (GLenum target, GLenum pname, GLint params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (GLAPIENTRY * PFNGLCOPYCONVOLUTIONFILTER1DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLCOPYCONVOLUTIONFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLGETCONVOLUTIONFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (GLAPIENTRY * PFNGLGETCONVOLUTIONPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETCONVOLUTIONPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETSEPARABLEFILTEREXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (GLAPIENTRY * PFNGLSEPARABLEFILTER2DEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (GLAPIENTRY * PFNGLCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (GLAPIENTRY * PFNGLCOLORTABLESGIPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (GLAPIENTRY * PFNGLCOPYCOLORTABLESGIPROC) (GLenum target, GLenum internalFormat, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPARAMETERFVSGIPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPARAMETERIVSGIPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLESGIPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (GLAPIENTRY * PFNGLPIXELTEXGENSGIXPROC) (GLenum mode);
typedef void (GLAPIENTRY * PFNGLPIXELTEXGENPARAMETERFSGISPROC) (GLenum target, GLfloat value);
typedef void (GLAPIENTRY * PFNGLPIXELTEXGENPARAMETERFVSGISPROC) (GLenum target, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLPIXELTEXGENPARAMETERISGISPROC) (GLenum target, GLint value);
typedef void (GLAPIENTRY * PFNGLPIXELTEXGENPARAMETERIVSGISPROC) (GLenum target, const GLint *value);
typedef void (GLAPIENTRY * PFNGLGETPIXELTEXGENPARAMETERFVSGISPROC) (GLenum target, GLfloat *value);
typedef void (GLAPIENTRY * PFNGLGETPIXELTEXGENPARAMETERIVSGISPROC) (GLenum target, GLint *value);
typedef void (GLAPIENTRY * PFNGLTEXIMAGE4DSGISPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei extent, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE4DSGISPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei extent, GLenum format, GLenum type, const void *pixels);
typedef void (GLAPIENTRY * PFNGLGENTEXTURESEXTPROC) (GLsizei n, GLuint *textures);
typedef void (GLAPIENTRY * PFNGLDELETETEXTURESEXTPROC) (GLsizei n, const GLuint *textures);
typedef void (GLAPIENTRY * PFNGLBINDTEXTUREEXTPROC) (GLenum target, GLuint texture);
typedef void (GLAPIENTRY * PFNGLPRIORITIZETEXTURESEXTPROC) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef GLboolean (GLAPIENTRY * PFNGLARETEXTURESRESIDENTEXTPROC) (GLsizei n, const GLuint *textures, GLboolean *residences);
typedef GLboolean (GLAPIENTRY * PFNGLISTEXTUREEXTPROC) (GLuint texture);
typedef void (GLAPIENTRY * PFNGLDETAILTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef void (GLAPIENTRY * PFNGLGETDETAILTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef void (GLAPIENTRY * PFNGLGETSHARPENTEXFUNCSGISPROC) (GLenum target, GLfloat *points);
typedef void (GLAPIENTRY * PFNGLSHARPENTEXFUNCSGISPROC) (GLenum target, GLsizei n, const GLfloat *points);
typedef void (GLAPIENTRY * PFNGLSAMPLEMASKSGISPROC) (GLclampf value, GLboolean invert);
typedef void (GLAPIENTRY * PFNGLSAMPLEPATTERNSGISPROC) (GLenum pattern);
typedef void (GLAPIENTRY * PFNGLARRAYELEMENTEXTPROC) (GLint i);
typedef void (GLAPIENTRY * PFNGLCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLDRAWARRAYSEXTPROC) (GLenum mode, GLint first, GLsizei count);
typedef void (GLAPIENTRY * PFNGLEDGEFLAGPOINTEREXTPROC) (GLsizei stride, GLsizei count, const GLboolean *pointer);
typedef void (GLAPIENTRY * PFNGLGETPOINTERVEXTPROC) (GLenum pname, GLvoid* *params);
typedef void (GLAPIENTRY * PFNGLINDEXPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLNORMALPOINTEREXTPROC) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLTEXCOORDPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLVERTEXPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONEXTPROC) (GLenum mode);
typedef void (GLAPIENTRY * PFNGLSPRITEPARAMETERFSGIXPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLSPRITEPARAMETERFVSGIXPROC) (GLenum pname, const GLfloat *param);
typedef void (GLAPIENTRY * PFNGLSPRITEPARAMETERISGIXPROC) (GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLSPRITEPARAMETERIVSGIXPROC) (GLenum pname, const GLint *param);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFEXTPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFVEXTPROC) (GLenum pname, const GLfloat *params);
typedef GLint (GLAPIENTRY * PFNGLGETINSTRUMENTSSGIXPROC) (void);
typedef void (GLAPIENTRY * PFNGLINSTRUMENTSBUFFERSGIXPROC) (GLsizei size, GLint *buf);
typedef GLint (GLAPIENTRY * PFNGLPOLLINSTRUMENTSSGIXPROC) (GLint *markerp);
typedef void (GLAPIENTRY * PFNGLREADINSTRUMENTSSGIXPROC) (GLint marker);
typedef void (GLAPIENTRY * PFNGLSTARTINSTRUMENTSSGIXPROC) (void);
typedef void (GLAPIENTRY * PFNGLSTOPINSTRUMENTSSGIXPROC) (GLint marker);
typedef void (GLAPIENTRY * PFNGLFRAMEZOOMSGIXPROC) (GLint factor);
typedef void (GLAPIENTRY * PFNGLTAGSAMPLEBUFFERSGIXPROC) (void);
typedef void (GLAPIENTRY * PFNGLREFERENCEPLANESGIXPROC) (const GLdouble *plane);
typedef void (GLAPIENTRY * PFNGLFLUSHRASTERSGIXPROC) (void);
typedef void (GLAPIENTRY * PFNGLCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data);
typedef void (GLAPIENTRY * PFNGLCOPYCOLORSUBTABLEEXTPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLHINTPGIPROC) (GLenum target, GLint mode);
typedef void (GLAPIENTRY * PFNGLCOLORTABLEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEEXTPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPARAMETERFVEXTPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum name, GLfloat *param);
typedef void (GLAPIENTRY * PFNGLGETLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum name, GLint *param);
typedef void (GLAPIENTRY * PFNGLLISTPARAMETERFSGIXPROC) (GLuint list, GLenum name, GLfloat param);
typedef void (GLAPIENTRY * PFNGLLISTPARAMETERFVSGIXPROC) (GLuint list, GLenum name, const GLfloat *param);
typedef void (GLAPIENTRY * PFNGLLISTPARAMETERISGIXPROC) (GLuint list, GLenum name, GLint param);
typedef void (GLAPIENTRY * PFNGLLISTPARAMETERIVSGIXPROC) (GLuint list, GLenum name, const GLint *param);
typedef void (GLAPIENTRY * PFNGLINDEXMATERIALEXTPROC) (GLenum face, GLenum mode);
typedef void (GLAPIENTRY * PFNGLINDEXFUNCEXTPROC) (GLenum func, GLfloat ref);
typedef void (GLAPIENTRY * PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
typedef void (GLAPIENTRY * PFNGLUNLOCKARRAYSEXTPROC) (void);
typedef void (GLAPIENTRY * PFNGLCULLPARAMETERDVEXTPROC) (GLenum pname, GLdouble* params);
typedef void (GLAPIENTRY * PFNGLCULLPARAMETERFVEXTPROC) (GLenum pname, GLfloat* params);
typedef void (GLAPIENTRY * PFNGLFRAGMENTCOLORMATERIALSGIXPROC) (GLenum face, GLenum mode);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTFSGIXPROC) (GLenum light, GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, const GLfloat * params);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTISGIXPROC) (GLenum light, GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, const GLint * params);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTMODELFSGIXPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTMODELFVSGIXPROC) (GLenum pname, const GLfloat * params);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTMODELISGIXPROC) (GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLFRAGMENTLIGHTMODELIVSGIXPROC) (GLenum pname, const GLint * params);
typedef void (GLAPIENTRY * PFNGLFRAGMENTMATERIALFSGIXPROC) (GLenum face, GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, const GLfloat * params);
typedef void (GLAPIENTRY * PFNGLFRAGMENTMATERIALISGIXPROC) (GLenum face, GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, const GLint * params);
typedef void (GLAPIENTRY * PFNGLGETFRAGMENTLIGHTFVSGIXPROC) (GLenum light, GLenum pname, GLfloat * params);
typedef void (GLAPIENTRY * PFNGLGETFRAGMENTLIGHTIVSGIXPROC) (GLenum light, GLenum pname, GLint * params);
typedef void (GLAPIENTRY * PFNGLGETFRAGMENTMATERIALFVSGIXPROC) (GLenum face, GLenum pname, GLfloat * params);
typedef void (GLAPIENTRY * PFNGLGETFRAGMENTMATERIALIVSGIXPROC) (GLenum face, GLenum pname, GLint * params);
typedef void (GLAPIENTRY * PFNGLLIGHTENVISGIXPROC) (GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLFOGCOORDFEXTPROC) (GLfloat coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDFVEXTPROC) (const GLfloat * coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDDEXTPROC) (GLdouble coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDDVEXTPROC) (const GLdouble * coord);
typedef void (GLAPIENTRY * PFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid * pointer);
typedef void (GLAPIENTRY * PFNGLBLENDFUNCSEPARATEEXTPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GLAPIENTRY * PFNGLBLENDFUNCSEPARATEINGRPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GLAPIENTRY * PFNGLADDSWAPHINTRECTWINPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLVERTEXWEIGHTFEXTPROC) (GLfloat weight);
typedef void (GLAPIENTRY * PFNGLVERTEXWEIGHTFVEXTPROC) (const GLfloat *weight);
typedef void (GLAPIENTRY * PFNGLVERTEXWEIGHTPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (GLAPIENTRY * PFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid * pointer);
typedef void (GLAPIENTRY * PFNGLCOMBINERPARAMETERFVNVPROC) (GLenum pname, const GLfloat * params);
typedef void (GLAPIENTRY * PFNGLCOMBINERPARAMETERFNVPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLCOMBINERPARAMETERIVNVPROC) (GLenum pname, const GLint * params);
typedef void (GLAPIENTRY * PFNGLCOMBINERPARAMETERINVPROC) (GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLCOMBINERINPUTNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef void (GLAPIENTRY * PFNGLCOMBINEROUTPUTNVPROC) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
typedef void (GLAPIENTRY * PFNGLFINALCOMBINERINPUTNVPROC) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
typedef void (GLAPIENTRY * PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat * params);
typedef void (GLAPIENTRY * PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint * params);
typedef void (GLAPIENTRY * PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLfloat * params);
typedef void (GLAPIENTRY * PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC) (GLenum stage, GLenum portion, GLenum pname, GLint * params);
typedef void (GLAPIENTRY * PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC) (GLenum variable, GLenum pname, GLfloat * params);
typedef void (GLAPIENTRY * PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC) (GLenum variable, GLenum pname, GLint * params);
typedef void (GLAPIENTRY * PFNGLRESIZEBUFFERSMESAPROC) (void);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2IMESAPROC) (GLint x, GLint y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2SMESAPROC) (GLshort x, GLshort y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2FMESAPROC) (GLfloat x, GLfloat y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2DMESAPROC) (GLdouble x, GLdouble y);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2IVMESAPROC) (const GLint *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2SVMESAPROC) (const GLshort *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2FVMESAPROC) (const GLfloat *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2DVMESAPROC) (const GLdouble *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3IMESAPROC) (GLint x, GLint y, GLint z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3SMESAPROC) (GLshort x, GLshort y, GLshort z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3FMESAPROC) (GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3DMESAPROC) (GLdouble x, GLdouble y, GLdouble z);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3IVMESAPROC) (const GLint *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3SVMESAPROC) (const GLshort *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3FVMESAPROC) (const GLfloat *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS3DVMESAPROC) (const GLdouble *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4SMESAPROC) (GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4FMESAPROC) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4DMESAPROC) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4IVMESAPROC) (const GLint *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4SVMESAPROC) (const GLshort *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4FVMESAPROC) (const GLfloat *p);
typedef void (GLAPIENTRY * PFNGLWINDOWPOS4DVMESAPROC) (const GLdouble *p);
typedef void (GLAPIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLLOADTRANSPOSEMATRIXDARBPROC) ( const GLdouble m[16] );
typedef void (GLAPIENTRY * PFNGLLOADTRANSPOSEMATRIXFARBPROC) ( const GLfloat m[16] );
typedef void (GLAPIENTRY * PFNGLMULTTRANSPOSEMATRIXDARBPROC) ( const GLdouble m[16] );
typedef void (GLAPIENTRY * PFNGLMULTTRANSPOSEMATRIXFARBPROC) ( const GLfloat m[16] );
typedef void (GLAPIENTRY * PFNGLSAMPLEPASSARBPROC) (GLenum pass);
typedef void (GLAPIENTRY * PFNGLSAMPLECOVERAGEARBPROC) (GLclampf value, GLboolean invert);

// GL_VERSION_1_2
typedef void (GLAPIENTRY * PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (GLAPIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// GL_VERSION_1_3
typedef void (GLAPIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s, GLfloat t);

// GL_VERSION_1_3_DEPRECATED
typedef void (GLAPIENTRY * PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);

// GL_VERSION_1_4
typedef void (GLAPIENTRY * PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GLAPIENTRY * PFNGLFOGCOORDFPROC) (GLfloat coord);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLOR3FVPROC) (const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLSECONDARYCOLORPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

// GL_VERSION_1_5
typedef void (GLAPIENTRY * PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void (GLAPIENTRY * PFNGLENDQUERYPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef GLvoid* (GLAPIENTRY * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean (GLAPIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);

// GL_VERSION_2_0
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
typedef void (GLAPIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
typedef void (GLAPIENTRY * PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (GLAPIENTRY * PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
typedef void (GLAPIENTRY * PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
typedef void (GLAPIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (GLAPIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
typedef void (GLAPIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (GLAPIENTRY * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (GLAPIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
typedef void (GLAPIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (GLAPIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (GLAPIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (GLAPIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (GLAPIENTRY * PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (GLAPIENTRY * PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void (GLAPIENTRY * PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
typedef GLint (GLAPIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GLAPIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (GLAPIENTRY * PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef GLint (GLAPIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef GLboolean (GLAPIENTRY * PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean (GLAPIENTRY * PFNGLISSHADERPROC) (GLuint shader);
typedef void (GLAPIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
typedef void (GLAPIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (GLAPIENTRY * PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void (GLAPIENTRY * PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (GLAPIENTRY * PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (GLAPIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (GLAPIENTRY * PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void (GLAPIENTRY * PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void (GLAPIENTRY * PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void (GLAPIENTRY * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GLAPIENTRY* PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GLAPIENTRY * PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GLAPIENTRY * PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GLAPIENTRY * PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

// GL_VERSION_2_1
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

// GL_ARB_point_parameters
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFARBPROC) (GLenum pname, GLfloat param);
typedef void (GLAPIENTRY * PFNGLPOINTPARAMETERFVARBPROC) (GLenum pname, const GLfloat *params);

// GL_ARB_window_pos
typedef void (GLAPIENTRY * PFNGLWINDOWPOS2IARBPROC) (GLint x, GLint y);

// GL_ARB_vertex_program
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB1SVARBPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x, GLshort y);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB2SVARBPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB3SVARBPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NBVARBPROC) (GLuint index, const GLbyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NIVARBPROC) (GLuint index, const GLint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NSVARBPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index, const GLubyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index, const GLuint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index, const GLushort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4BVARBPROC) (GLuint index, const GLbyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4DVARBPROC) (GLuint index, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4FVARBPROC) (GLuint index, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4IVARBPROC) (GLuint index, const GLint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4SVARBPROC) (GLuint index, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UBVARBPROC) (GLuint index, const GLubyte *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4UIVARBPROC) (GLuint index, const GLuint *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIB4USVARBPROC) (GLuint index, const GLushort *v);
typedef void (GLAPIENTRY * PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (GLAPIENTRY * PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (GLAPIENTRY * PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void (GLAPIENTRY * PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef void (GLAPIENTRY * PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void (GLAPIENTRY * PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat *params);
typedef GLboolean (GLAPIENTRY * PFNGLISPROGRAMARBPROC) (GLuint program);

// GL_ARB_vertex_buffer_object
typedef void (GLAPIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (GLAPIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (GLAPIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef GLboolean (GLAPIENTRY * PFNGLISBUFFERARBPROC) (GLuint buffer);
typedef void (GLAPIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
typedef void (GLAPIENTRY * PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
typedef GLvoid* (GLAPIENTRY * PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (GLAPIENTRY * PFNGLUNMAPBUFFERARBPROC) (GLenum target);

// GL_ARB_occlusion_query
typedef void (GLAPIENTRY * PFNGLGENQUERIESARBPROC) (GLsizei n, GLuint *ids);
typedef void (GLAPIENTRY * PFNGLDELETEQUERIESARBPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (GLAPIENTRY * PFNGLISQUERYARBPROC) (GLuint id);
typedef void (GLAPIENTRY * PFNGLBEGINQUERYARBPROC) (GLenum target, GLuint id);
typedef void (GLAPIENTRY * PFNGLENDQUERYARBPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLGETQUERYIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETQUERYOBJECTIVARBPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLGETQUERYOBJECTUIVARBPROC) (GLuint id, GLenum pname, GLuint *params);

// GL_ARB_map_buffer_range
typedef GLvoid* (GLAPIENTRY * PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (GLAPIENTRY * PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);

// GL_ARB_copy_buffer
typedef void (GLAPIENTRY * PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

// GL_EXT_stencil_two_side
typedef void (GLAPIENTRY * PFNGLACTIVESTENCILFACEEXTPROC) (GLenum face);

// GL_ATI_separate_stencil
typedef void (GLAPIENTRY * PFNGLSTENCILOPSEPARATEATIPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (GLAPIENTRY * PFNGLSTENCILFUNCSEPARATEATIPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);

// GL_EXT_framebuffer_object
typedef GLboolean (GLAPIENTRY * PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (GLAPIENTRY * PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (GLAPIENTRY * PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (GLAPIENTRY * PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (GLAPIENTRY * PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (GLAPIENTRY * PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (GLAPIENTRY * PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (GLAPIENTRY * PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (GLAPIENTRY * PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (GLAPIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (GLAPIENTRY * PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (GLAPIENTRY * PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);

// GL_EXT_provoking_vertex
typedef void (GLAPIENTRY * PFNGLPROVOKINGVERTEXEXTPROC) (GLenum mode);

// GL_ARB_provoking_vertex
typedef void (GLAPIENTRY * PFNGLPROVOKINGVERTEXPROC) (GLenum mode);

// OpenGL 1.2 enumerants, to allow glean to be compiled on OpenGL 1.1 systems.
// (This odd workaround is needed to handle problems with some copies of
// glext.h that are floating around the net.)

#ifndef GL_PACK_SKIP_IMAGES
#define GL_PACK_SKIP_IMAGES			0x806B
#endif
#ifndef GL_PACK_IMAGE_HEIGHT
#define GL_PACK_IMAGE_HEIGHT			0x806C
#endif
#ifndef GL_UNPACK_SKIP_IMAGES
#define GL_UNPACK_SKIP_IMAGES			0x806D
#endif
#ifndef GL_UNPACK_IMAGE_HEIGHT
#define GL_UNPACK_IMAGE_HEIGHT			0x806E
#endif
#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D				0x806F
#endif
#ifndef GL_PROXY_TEXTURE_3D
#define GL_PROXY_TEXTURE_3D			0x8070
#endif
#ifndef GL_TEXTURE_DEPTH
#define GL_TEXTURE_DEPTH			0x8071
#endif
#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R			0x8072
#endif
#ifndef GL_MAX_3D_TEXTURE_SIZE
#define GL_MAX_3D_TEXTURE_SIZE			0x8073
#endif
#ifndef GL_TEXTURE_BINDING_3D
#define GL_TEXTURE_BINDING_3D			0x806A
#endif
#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL			0x803A
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE			0x812F
#endif
#ifndef GL_MAX_ELEMENTS_VERTICES
#define GL_MAX_ELEMENTS_VERTICES		0x80E8
#endif
#ifndef GL_MAX_ELEMENTS_INDICES
#define GL_MAX_ELEMENTS_INDICES			0x80E9
#endif
#ifndef GL_BGR
#define GL_BGR					0x80E0
#endif
#ifndef GL_BGRA
#define GL_BGRA					0x80E1
#endif
#ifndef GL_UNSIGNED_BYTE_3_3_2
#define GL_UNSIGNED_BYTE_3_3_2			0x8032
#endif
#ifndef GL_UNSIGNED_BYTE_2_3_3_REV
#define GL_UNSIGNED_BYTE_2_3_3_REV		0x8362
#endif
#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5			0x8363
#endif
#ifndef GL_UNSIGNED_SHORT_5_6_5_REV
#define GL_UNSIGNED_SHORT_5_6_5_REV		0x8364
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4
#define GL_UNSIGNED_SHORT_4_4_4_4		0x8033
#endif
#ifndef GL_UNSIGNED_SHORT_4_4_4_4_REV
#define GL_UNSIGNED_SHORT_4_4_4_4_REV		0x8365
#endif
#ifndef GL_UNSIGNED_SHORT_5_5_5_1
#define GL_UNSIGNED_SHORT_5_5_5_1		0x8034
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#define GL_UNSIGNED_SHORT_1_5_5_5_REV		0x8366
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8
#define GL_UNSIGNED_INT_8_8_8_8			0x8035
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV		0x8367
#endif
#ifndef GL_UNSIGNED_INT_10_10_10_2
#define GL_UNSIGNED_INT_10_10_10_2		0x8036
#endif
#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV		0x8368
#endif
#ifndef GL_LIGHT_MODEL_COLOR_CONTROL
#define GL_LIGHT_MODEL_COLOR_CONTROL		0x81F8
#endif
#ifndef GL_SINGLE_COLOR
#define GL_SINGLE_COLOR				0x81F9
#endif
#ifndef GL_SEPARATE_SPECULAR_COLOR
#define GL_SEPARATE_SPECULAR_COLOR		0x81FA
#endif
#ifndef GL_TEXTURE_MIN_LOD
#define GL_TEXTURE_MIN_LOD			0x813A
#endif
#ifndef GL_TEXTURE_MAX_LOD
#define GL_TEXTURE_MAX_LOD			0x813B
#endif
#ifndef GL_TEXTURE_BASE_LEVEL
#define GL_TEXTURE_BASE_LEVEL			0x813C
#endif
#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL			0x813D
#endif
#ifndef GL_SMOOTH_POINT_SIZE_RANGE
#define GL_SMOOTH_POINT_SIZE_RANGE		0x0B12
#endif
#ifndef GL_SMOOTH_POINT_SIZE_GRANULARITY
#define GL_SMOOTH_POINT_SIZE_GRANULARITY	0x0B13
#endif
#ifndef GL_SMOOTH_LINE_WIDTH_RANGE
#define GL_SMOOTH_LINE_WIDTH_RANGE		0x0B22
#endif
#ifndef GL_SMOOTH_LINE_WIDTH_GRANULARITY
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY	0x0B23
#endif
#ifndef GL_ALIASED_POINT_SIZE_RANGE
#define GL_ALIASED_POINT_SIZE_RANGE		0x846D
#endif
#ifndef GL_ALIASED_LINE_WIDTH_RANGE
#define GL_ALIASED_LINE_WIDTH_RANGE		0x846E
#endif
#ifndef GL_COLOR_TABLE
#define GL_COLOR_TABLE				0x80D0
#endif
#ifndef GL_POST_CONVOLUTION_COLOR_TABLE
#define GL_POST_CONVOLUTION_COLOR_TABLE		0x80D1
#endif
#ifndef GL_POST_COLOR_MATRIX_COLOR_TABLE
#define GL_POST_COLOR_MATRIX_COLOR_TABLE	0x80D2
#endif
#ifndef GL_PROXY_COLOR_TABLE
#define GL_PROXY_COLOR_TABLE			0x80D3
#endif
#ifndef GL_PROXY_POST_CONVOLUTION_COLOR_TABLE
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE	0x80D4
#endif
#ifndef GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE	0x80D5
#endif
#ifndef GL_COLOR_TABLE_SCALE
#define GL_COLOR_TABLE_SCALE			0x80D6
#endif
#ifndef GL_COLOR_TABLE_BIAS
#define GL_COLOR_TABLE_BIAS			0x80D7
#endif
#ifndef GL_COLOR_TABLE_FORMAT
#define GL_COLOR_TABLE_FORMAT			0x80D8
#endif
#ifndef GL_COLOR_TABLE_WIDTH
#define GL_COLOR_TABLE_WIDTH			0x80D9
#endif
#ifndef GL_COLOR_TABLE_RED_SIZE
#define GL_COLOR_TABLE_RED_SIZE			0x80DA
#endif
#ifndef GL_COLOR_TABLE_GREEN_SIZE
#define GL_COLOR_TABLE_GREEN_SIZE		0x80DB
#endif
#ifndef GL_COLOR_TABLE_BLUE_SIZE
#define GL_COLOR_TABLE_BLUE_SIZE		0x80DC
#endif
#ifndef GL_COLOR_TABLE_ALPHA_SIZE
#define GL_COLOR_TABLE_ALPHA_SIZE		0x80DD
#endif
#ifndef GL_COLOR_TABLE_LUMINANCE_SIZE
#define GL_COLOR_TABLE_LUMINANCE_SIZE		0x80DE
#endif
#ifndef GL_COLOR_TABLE_INTENSITY_SIZE
#define GL_COLOR_TABLE_INTENSITY_SIZE		0x80DF
#endif
#ifndef GL_CONVOLUTION_1D
#define GL_CONVOLUTION_1D			0x8010
#endif
#ifndef GL_CONVOLUTION_2D
#define GL_CONVOLUTION_2D			0x8011
#endif
#ifndef GL_SEPARABLE_2D
#define GL_SEPARABLE_2D				0x8012
#endif
#ifndef GL_CONVOLUTION_BORDER_MODE
#define GL_CONVOLUTION_BORDER_MODE		0x8013
#endif
#ifndef GL_CONVOLUTION_FILTER_SCALE
#define GL_CONVOLUTION_FILTER_SCALE		0x8014
#endif
#ifndef GL_CONVOLUTION_FILTER_BIAS
#define GL_CONVOLUTION_FILTER_BIAS		0x8015
#endif
#ifndef GL_REDUCE
#define GL_REDUCE				0x8016
#endif
#ifndef GL_CONVOLUTION_FORMAT
#define GL_CONVOLUTION_FORMAT			0x8017
#endif
#ifndef GL_CONVOLUTION_WIDTH
#define GL_CONVOLUTION_WIDTH			0x8018
#endif
#ifndef GL_CONVOLUTION_HEIGHT
#define GL_CONVOLUTION_HEIGHT			0x8019
#endif
#ifndef GL_MAX_CONVOLUTION_WIDTH
#define GL_MAX_CONVOLUTION_WIDTH		0x801A
#endif
#ifndef GL_MAX_CONVOLUTION_HEIGHT
#define GL_MAX_CONVOLUTION_HEIGHT		0x801B
#endif
#ifndef GL_POST_CONVOLUTION_RED_SCALE
#define GL_POST_CONVOLUTION_RED_SCALE		0x801C
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_SCALE
#define GL_POST_CONVOLUTION_GREEN_SCALE		0x801D
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_SCALE
#define GL_POST_CONVOLUTION_BLUE_SCALE		0x801E
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_SCALE
#define GL_POST_CONVOLUTION_ALPHA_SCALE		0x801F
#endif
#ifndef GL_POST_CONVOLUTION_RED_BIAS
#define GL_POST_CONVOLUTION_RED_BIAS		0x8020
#endif
#ifndef GL_POST_CONVOLUTION_GREEN_BIAS
#define GL_POST_CONVOLUTION_GREEN_BIAS		0x8021
#endif
#ifndef GL_POST_CONVOLUTION_BLUE_BIAS
#define GL_POST_CONVOLUTION_BLUE_BIAS		0x8022
#endif
#ifndef GL_POST_CONVOLUTION_ALPHA_BIAS
#define GL_POST_CONVOLUTION_ALPHA_BIAS		0x8023
#endif
#ifndef GL_CONSTANT_BORDER
#define GL_CONSTANT_BORDER			0x8151
#endif
#ifndef GL_REPLICATE_BORDER
#define GL_REPLICATE_BORDER			0x8153
#endif
#ifndef GL_CONVOLUTION_BORDER_COLOR
#define GL_CONVOLUTION_BORDER_COLOR		0x8154
#endif
#ifndef GL_COLOR_MATRIX
#define GL_COLOR_MATRIX				0x80B1
#endif
#ifndef GL_COLOR_MATRIX_STACK_DEPTH
#define GL_COLOR_MATRIX_STACK_DEPTH		0x80B2
#endif
#ifndef GL_MAX_COLOR_MATRIX_STACK_DEPTH
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH		0x80B3
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_SCALE
#define GL_POST_COLOR_MATRIX_RED_SCALE		0x80B4
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_SCALE
#define GL_POST_COLOR_MATRIX_GREEN_SCALE	0x80B5
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_SCALE
#define GL_POST_COLOR_MATRIX_BLUE_SCALE		0x80B6
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_SCALE
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE	0x80B7
#endif
#ifndef GL_POST_COLOR_MATRIX_RED_BIAS
#define GL_POST_COLOR_MATRIX_RED_BIAS		0x80B8
#endif
#ifndef GL_POST_COLOR_MATRIX_GREEN_BIAS
#define GL_POST_COLOR_MATRIX_GREEN_BIAS		0x80B9
#endif
#ifndef GL_POST_COLOR_MATRIX_BLUE_BIAS
#define GL_POST_COLOR_MATRIX_BLUE_BIAS		0x80BA
#endif
#ifndef GL_POST_COLOR_MATRIX_ALPHA_BIAS
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS		0x80BB
#endif
#ifndef GL_HISTOGRAM
#define GL_HISTOGRAM				0x8024
#endif
#ifndef GL_PROXY_HISTOGRAM
#define GL_PROXY_HISTOGRAM			0x8025
#endif
#ifndef GL_HISTOGRAM_WIDTH
#define GL_HISTOGRAM_WIDTH			0x8026
#endif
#ifndef GL_HISTOGRAM_FORMAT
#define GL_HISTOGRAM_FORMAT			0x8027
#endif
#ifndef GL_HISTOGRAM_RED_SIZE
#define GL_HISTOGRAM_RED_SIZE			0x8028
#endif
#ifndef GL_HISTOGRAM_GREEN_SIZE
#define GL_HISTOGRAM_GREEN_SIZE			0x8029
#endif
#ifndef GL_HISTOGRAM_BLUE_SIZE
#define GL_HISTOGRAM_BLUE_SIZE			0x802A
#endif
#ifndef GL_HISTOGRAM_ALPHA_SIZE
#define GL_HISTOGRAM_ALPHA_SIZE			0x802B
#endif
#ifndef GL_HISTOGRAM_LUMINANCE_SIZE
#define GL_HISTOGRAM_LUMINANCE_SIZE		0x802C
#endif
#ifndef GL_HISTOGRAM_SINK
#define GL_HISTOGRAM_SINK			0x802D
#endif
#ifndef GL_MINMAX
#define GL_MINMAX				0x802E
#endif
#ifndef GL_MINMAX_FORMAT
#define GL_MINMAX_FORMAT			0x802F
#endif
#ifndef GL_MINMAX_SINK
#define GL_MINMAX_SINK				0x8030
#endif
#ifndef GL_TABLE_TOO_LARGE
#define GL_TABLE_TOO_LARGE			0x8031
#endif
#ifndef GL_BLEND_EQUATION
#define GL_BLEND_EQUATION			0x8009
#endif
#ifndef GL_MIN
#define GL_MIN					0x8007
#endif
#ifndef GL_MAX
#define GL_MAX					0x8008
#endif
#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD				0x8006
#endif
#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT			0x800A
#endif
#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT		0x800B
#endif
#ifndef GL_BLEND_COLOR
#define	GL_BLEND_COLOR				0x8005
#endif

// Extension enumerants, in case they're not defined in glext.h.

#ifndef GL_DOT3_RGB_EXT
#define GL_DOT3_RGB_EXT				0x8740
#endif
#ifndef GL_DOT3_RGBA_EXT
#define GL_DOT3_RGBA_EXT			0x8741
#endif
#ifndef GL_DOT3_RGB_ARB
#define GL_DOT3_RGB_ARB				0x86AE
#endif
#ifndef GL_DOT3_RGBA_ARB
#define GL_DOT3_RGBA_ARB			0x86AF
#endif


#ifndef GL_VERSION_1_2
// OpenGL 1.2 function pointer types, to allow glean to
// be compiled on OpenGL 1.1 systems.
typedef void (GLAPIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (GLAPIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (GLAPIENTRY * PFNGLCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (GLAPIENTRY * PFNGLCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (GLAPIENTRY * PFNGLCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLCOPYCOLORSUBTABLEPROC) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLCOPYCOLORTABLEPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPROC) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETCOLORTABLEPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
typedef void (GLAPIENTRY * PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY * PFNGLHISTOGRAMPROC) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (GLAPIENTRY * PFNGLRESETHISTOGRAMPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLGETHISTOGRAMPROC) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (GLAPIENTRY * PFNGLGETHISTOGRAMPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETHISTOGRAMPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLMINMAXPROC) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (GLAPIENTRY * PFNGLRESETMINMAXPROC) (GLenum target);
typedef void (GLAPIENTRY * PFNGLGETMINMAXPROC) (GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values);
typedef void (GLAPIENTRY * PFNGLGETMINMAXPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETMINMAXPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERFPROC) (GLenum target, GLenum pname, GLfloat params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERIPROC) (GLenum target, GLenum pname, GLint params);
typedef void (GLAPIENTRY * PFNGLCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, const GLint *params);
typedef void (GLAPIENTRY * PFNGLCOPYCONVOLUTIONFILTER1DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY * PFNGLCOPYCONVOLUTIONFILTER2DPROC) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY * PFNGLGETCONVOLUTIONFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (GLAPIENTRY * PFNGLGETCONVOLUTIONPARAMETERFVPROC) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY * PFNGLGETCONVOLUTIONPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY * PFNGLSEPARABLEFILTER2DPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (GLAPIENTRY * PFNGLGETSEPARABLEFILTERPROC) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (GLAPIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void (GLAPIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
#endif
} // namespace GLEAN


#endif // __glwrap_h__
