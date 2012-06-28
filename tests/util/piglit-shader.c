/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/stat.h>
#include <errno.h>

#include "piglit-util-gl-common.h"

void piglit_get_glsl_version(bool *es, int* major, int* minor)
{
	bool es_local;
	int major_local;
	int minor_local;

	const char *version_string;
	int c; /* scanf count */

	version_string = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
	es_local = strncmp("OpenGL ES", version_string, 10) == 0;
	if (es_local) {
		c = sscanf(version_string,
		           "OpenGL ES GLSL ES %i.%i",
		           &major_local,
		           &minor_local);
	} else {
		c = sscanf(version_string,
		           "%i.%i",
		           &major_local,
		           &minor_local);
	}
	assert(c == 2);

	/* Write outputs. */
	if (es != NULL)
		*es = es_local;
	if (major != NULL)
		*major = major_local;
	if (minor != NULL)
		*minor = minor_local;
}

/**
 * Convenience function to compile a GLSL shader from a file.
 */
GLuint
piglit_compile_shader(GLenum target, char *filename)
{
	GLuint prog;
	struct stat st;
	int err;
	GLchar *prog_string;
	FILE *f;
	const char *source_dir;
	char filename_with_path[FILENAME_MAX];

	source_dir = getenv("PIGLIT_SOURCE_DIR");
	if (source_dir == NULL) {
		source_dir = SOURCE_DIR;
	}

	snprintf(filename_with_path, FILENAME_MAX - 1,
		 "%s/tests/%s", source_dir, filename);
	filename_with_path[FILENAME_MAX - 1] = 0;

	err = stat(filename_with_path, &st);
	if (err == -1) {
		fprintf(stderr, "Couldn't stat program %s: %s\n", filename_with_path, strerror(errno));
		fprintf(stderr, "You can override the source dir by setting the PIGLIT_SOURCE_DIR environment variable.\n");
		exit(1);
	}

	prog_string = malloc(st.st_size + 1);
	if (prog_string == NULL) {
		fprintf(stderr, "malloc\n");
		exit(1);
	}

	f = fopen(filename_with_path, "r");
	if (f == NULL) {
		fprintf(stderr, "Couldn't open program: %s\n", strerror(errno));
		exit(1);
	}
	fread(prog_string, 1, st.st_size, f);
	prog_string[st.st_size] = '\0';
	fclose(f);

	prog = piglit_compile_shader_text(target, prog_string);

	free(prog_string);

	return prog;
}

/** Return a string name for a shader target enum */
static const char *
shader_name(GLenum target)
{
   switch (target) {
   case GL_VERTEX_SHADER:
      return "vertex";
#if defined USE_OPENGL
   case GL_GEOMETRY_SHADER:
      return "geometry";
#endif
   case GL_FRAGMENT_SHADER:
      return "fragment";
   default:
      assert(!"Unexpected shader target in shader_name()");
   }
}

/**
 * Convenience function to compile a GLSL shader.
 */
GLuint
piglit_compile_shader_text(GLenum target, const char *text)
{
	GLuint prog;
	GLint ok;

	piglit_require_GLSL();

	prog = piglit_CreateShader(target);
	piglit_ShaderSource(prog, 1, (const GLchar **) &text, NULL);
	piglit_CompileShader(prog);

	piglit_GetShaderiv(prog, GL_COMPILE_STATUS, &ok);

	{
		GLchar *info;
		GLint size;

		piglit_GetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		piglit_GetShaderInfoLog(prog, size, NULL, info);
		if (!ok) {
			fprintf(stderr, "Failed to compile %s shader: %s\n",
				shader_name(target),
				info);
			piglit_DeleteShader(prog);
			prog = 0;
		}
		else if (0) {
			/* Enable this to get extra compilation info.
			 * Even if there's no compilation errors, the info
			 * log may have some remarks.
			 */
			fprintf(stderr, "Shader compiler warning: %s\n", info);
		}
		free(info);
	}

	return prog;
}

static GLboolean
link_check_status(GLint prog, FILE *output)
{
	GLchar *info = NULL;
	GLint size;
	GLint ok;

	piglit_require_GLSL();

	piglit_GetProgramiv(prog, GL_LINK_STATUS, &ok);

	/* Some drivers return a size of 1 for an empty log.  This is the size
	 * of a log that contains only a terminating NUL character.
	 */
	piglit_GetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
	if (size > 1) {
		info = malloc(size);
		piglit_GetProgramInfoLog(prog, size, NULL, info);
	}

	if (!ok) {
		fprintf(output, "Failed to link: %s\n",
			(info != NULL) ? info : "<empty log>");
	}
	else if (0 && info != NULL) {
		/* Enable this to get extra linking info.
		 * Even if there's no link errors, the info log may
		 * have some remarks.
		 */
		printf("Linker warning: %s\n", info);
	}

	free(info);

	return ok;
}

GLboolean
piglit_link_check_status(GLint prog)
{
	return link_check_status(prog, stderr);
}

/**
 * Check link status
 *
 * Similar to piglit_link_check_status except it logs error messages
 * to standard output instead of standard error.  This is useful for
 * tests that want to produce negative link results.
 *
 * \sa piglit_link_check_status
 */
GLboolean
piglit_link_check_status_quiet(GLint prog)
{
	return link_check_status(prog, stdout);
}


GLint piglit_link_simple_program(GLint vs, GLint fs)
{
	GLint prog;

	piglit_require_GLSL();

	prog = piglit_CreateProgram();
	if (vs)
		piglit_AttachShader(prog, vs);
	if (fs)
		piglit_AttachShader(prog, fs);
	piglit_LinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		prog = 0;
	}

	return prog;
}
