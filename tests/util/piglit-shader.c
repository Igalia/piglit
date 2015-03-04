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

#include <errno.h>

#include "piglit-util-gl.h"

void piglit_get_glsl_version(bool *es, int* major, int* minor)
{
	bool es_local;
	int major_local;
	int minor_local;

	const char *version_string;
	int c; /* scanf count */

	version_string = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
	es_local = strncmp("OpenGL ES", version_string, 9) == 0;
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
piglit_compile_shader(GLenum target, const char *filename)
{
	GLuint prog;
	GLchar *prog_string;
	const char *source_dir;
	char filename_with_path[FILENAME_MAX];

	source_dir = getenv("PIGLIT_SOURCE_DIR");
	if (source_dir == NULL) {
		source_dir = SOURCE_DIR;
	}

	snprintf(filename_with_path, FILENAME_MAX - 1,
		 "%s/tests/%s", source_dir, filename);
	filename_with_path[FILENAME_MAX - 1] = 0;

	prog_string = piglit_load_text_file(filename_with_path, NULL);
	if (!prog_string) {
		fprintf(stderr, "Couldn't read shader %s: %s\n",
			filename_with_path, strerror(errno));
		fprintf(stderr, "You can override the source dir by setting the"
			" PIGLIT_SOURCE_DIR environment variable.\n");
		exit(1);
	}

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
   case GL_TESS_CONTROL_SHADER:
      return "tessellation control";
   case GL_TESS_EVALUATION_SHADER:
      return "tessellation evaluation";
   case GL_GEOMETRY_SHADER:
      return "geometry";
   case GL_FRAGMENT_SHADER:
      return "fragment";
   case GL_COMPUTE_SHADER:
      return "compute";
   default:
      assert(!"Unexpected shader target in shader_name()");
   }

   return "error";
}

/**
 * Convenience function to compile a GLSL shader.
 */
GLuint
piglit_compile_shader_text_nothrow(GLenum target, const char *text)
{
	GLuint prog;
	GLint ok;

	piglit_require_GLSL();

	prog = glCreateShader(target);
	glShaderSource(prog, 1, (const GLchar **) &text, NULL);
	glCompileShader(prog);

	glGetShaderiv(prog, GL_COMPILE_STATUS, &ok);

	{
		GLchar *info;
		GLint size;

		glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &size);
		info = malloc(size);

		glGetShaderInfoLog(prog, size, NULL, info);
		if (!ok) {
			fprintf(stderr, "Failed to compile %s shader: %s\n",
				shader_name(target),
				info);

			fprintf(stderr, "source:\n%s", text);
			glDeleteShader(prog);
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

/**
 * Convenience function to compile a GLSL shader.  Throws PIGLIT_FAIL
 * on error terminating the program.
 */
GLuint
piglit_compile_shader_text(GLenum target, const char *text)
{
        GLuint shader = piglit_compile_shader_text_nothrow(target, text);

        if (!shader)
                piglit_report_result(PIGLIT_FAIL);

        return shader;
}

static GLboolean
link_check_status(GLint prog, FILE *output)
{
	GLchar *info = NULL;
	GLint size;
	GLint ok;

	piglit_require_GLSL();

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);

	/* Some drivers return a size of 1 for an empty log.  This is the size
	 * of a log that contains only a terminating NUL character.
	 */
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
	if (size > 1) {
		info = malloc(size);
		glGetProgramInfoLog(prog, size, NULL, info);
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

	prog = glCreateProgram();
	if (vs)
		glAttachShader(prog, vs);
	if (fs)
		glAttachShader(prog, fs);

	/* If the shaders reference piglit_vertex or piglit_tex, bind
	 * them to some fixed attribute locations so they can be used
	 * with piglit_draw_rect_tex() in GLES.
	 */
	glBindAttribLocation(prog, PIGLIT_ATTRIB_POS, "piglit_vertex");
	glBindAttribLocation(prog, PIGLIT_ATTRIB_TEX, "piglit_texcoord");

	glLinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		prog = 0;
	}

	return prog;
}


/**
 * Builds a program from optional VS and FS sources, but does not link
 * it.  If there is a compile failure, the test is terminated.
 */
GLuint
piglit_build_simple_program_unlinked(const char *vs_source,
				     const char *fs_source)
{
	GLuint prog;

	piglit_require_GLSL();
	prog = glCreateProgram();
	if (vs_source) {
		GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
						       vs_source);
		glAttachShader(prog, vs);
		glDeleteShader(vs);
	}
	if (fs_source) {
		GLuint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						       fs_source);
		glAttachShader(prog, fs);
		glDeleteShader(fs);
	}
	return prog;
}


/**
 * Builds and links a program from optional VS and FS sources,
 * throwing PIGLIT_FAIL on error.
 */
GLint
piglit_build_simple_program(const char *vs_source, const char *fs_source)
{
	GLuint vs = 0, fs = 0, prog;

	if (vs_source) {
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	}

	if (fs_source) {
		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	}

	prog = piglit_link_simple_program(vs, fs);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	if (fs)
		glDeleteShader(fs);
	if (vs)
		glDeleteShader(vs);

	return prog;
}

GLint piglit_link_simple_program_multiple_shaders(GLint shader1, ...)
{
	va_list ap;
	GLint prog, sh;

	piglit_require_GLSL();

	prog = glCreateProgram();

	va_start(ap, shader1);
	sh = shader1;

	while (sh != 0) {
		glAttachShader(prog, sh);
		sh = va_arg(ap, GLint);
	}

	va_end(ap);

	/* If the shaders reference piglit_vertex or piglit_tex, bind
	 * them to some fixed attribute locations so they can be used
	 * with piglit_draw_rect_tex() in GLES.
	 */
	glBindAttribLocation(prog, PIGLIT_ATTRIB_POS, "piglit_vertex");
	glBindAttribLocation(prog, PIGLIT_ATTRIB_TEX, "piglit_texcoord");

	glLinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		prog = 0;
	}

	return prog;
}

GLint
piglit_build_simple_program_unlinked_multiple_shaders_v(GLenum target1,
						        const char *source1,
						        va_list ap)
{
	GLuint prog;
	GLenum target;
	const char *source;

	piglit_require_GLSL();
	prog = glCreateProgram();

	source = source1;
	target = target1;

	while (target != 0) {
		/* do not compile/attach a NULL shader */
		if (source) {
			GLuint shader = piglit_compile_shader_text(target,
								   source);

			glAttachShader(prog, shader);
			glDeleteShader(shader);
		}

		target  = va_arg(ap, GLenum);
		if (target != 0)
			source = va_arg(ap, char*);
	}

	return prog;
}

/**
 * Builds and links a program from optional sources,  but does not link
 * it. The last target must be 0. If there is a compile failure,
 * the test is terminated.
 *
 * example:
 * piglit_build_simple_program_unlinked_multiple_shaders(
 *				GL_VERTEX_SHADER,   vs,
 *				GL_GEOMETRY_SHADER, gs,
 *				GL_FRAGMENT_SHADER, fs,
 *				0);
 */
GLint
piglit_build_simple_program_unlinked_multiple_shaders(GLenum target1,
						      const char *source1,
						      ...)
{
	GLuint prog;
	va_list ap;

	va_start(ap, source1);

	prog = piglit_build_simple_program_unlinked_multiple_shaders_v(target1,
								       source1,
								       ap);
	va_end(ap);

	return prog;
}

/**
 * Builds and links a program from optional sources, throwing
 * PIGLIT_FAIL on error. The last target must be 0.
 */
GLint
piglit_build_simple_program_multiple_shaders(GLenum target1,
					    const char *source1,
					    ...)
{
	va_list ap;
	GLuint prog;

	va_start(ap, source1);

	prog = piglit_build_simple_program_unlinked_multiple_shaders_v(target1,
								       source1,
								       ap);

	va_end(ap);

	/* If the shaders reference piglit_vertex or piglit_tex, bind
	 * them to some fixed attribute locations so they can be used
	 * with piglit_draw_rect_tex() in GLES.
	 */
	glBindAttribLocation(prog, PIGLIT_ATTRIB_POS, "piglit_vertex");
	glBindAttribLocation(prog, PIGLIT_ATTRIB_TEX, "piglit_texcoord");

	glLinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		prog = 0;
		piglit_report_result(PIGLIT_FAIL);
	}

	return prog;
}

void
piglit_require_GLSL(void)
{
	if (piglit_get_gl_version() < 20
	    && !(piglit_is_extension_supported("GL_ARB_shader_objects")
	         && piglit_is_extension_supported("GL_ARB_shading_language_100"))) {
		printf("GLSL not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

void
piglit_require_GLSL_version(int version)
{
	bool es;
	int major, minor;

	piglit_require_GLSL();

	piglit_get_glsl_version(&es, &major, &minor);

	if (es || 100 * major + minor < version) {
		printf("GLSL %d.%d not supported.\n",
		       version / 100, version % 100);
		piglit_report_result(PIGLIT_SKIP);
	}
}

void
piglit_require_vertex_shader(void)
{
	if (piglit_get_gl_version() < 20
	    && !(piglit_is_extension_supported("GL_ARB_shader_objects")
		 && piglit_is_extension_supported("GL_ARB_vertex_shader"))) {
		printf("GLSL vertex shaders are not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

void
piglit_require_fragment_shader(void)
{
	if (piglit_get_gl_version() < 20
	    && !(piglit_is_extension_supported("GL_ARB_shader_objects")
		 && piglit_is_extension_supported("GL_ARB_fragment_shader"))) {
		printf("GLSL fragment shaders are not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

/* Same function as link_check_status but for program pipeline */
static GLboolean
program_pipeline_check_status(GLuint pipeline, FILE *output)
{
	GLchar *info = NULL;
	GLint size;
	GLint ok;

	piglit_require_extension("GL_ARB_separate_shader_objects");

	glValidateProgramPipeline(pipeline);
	glGetProgramPipelineiv(pipeline, GL_VALIDATE_STATUS, &ok);

	/* Some drivers return a size of 1 for an empty log.  This is the size
	 * of a log that contains only a terminating NUL character.
	 */
	glGetProgramPipelineiv(pipeline, GL_INFO_LOG_LENGTH, &size);
	if (size > 1) {
		info = malloc(size);
		glGetProgramPipelineInfoLog(pipeline, size, NULL, info);
	}

	if (!ok) {
		fprintf(output, "Failed to validate the pipeline: %s\n",
			(info != NULL) ? info : "<empty log>");
	}
	else if (0 && info != NULL) {
		/* Enable this to get extra linking info.
		 * Even if there's no link errors, the info log may
		 * have some remarks.
		 */
		printf("Pipeline validation warning: %s\n", info);
	}

	free(info);

	return ok;
}

GLboolean
piglit_program_pipeline_check_status(GLuint pipeline)
{
	return program_pipeline_check_status(pipeline, stderr);
}

GLboolean
piglit_program_pipeline_check_status_quiet(GLuint pipeline)
{
	return program_pipeline_check_status(pipeline, stdout);
}
