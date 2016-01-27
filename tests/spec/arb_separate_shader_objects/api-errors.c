/*
 * Copyright © 2014 Intel Corporation
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
 *
 */

/**
 * \file api-errors.c
 * Verify miscelaneous API error conditions from the specification.
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static bool
relink_program_created_by_glCreateShaderProgram(void)
{
	static const char *code = "void main() { gl_Position = vec4(0); }";
	GLuint prog = 0;
	GLuint vs = 0;
	bool pass = true;

	prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
				      (const GLchar * const*) &code);
	if (!piglit_link_check_status(prog)) {
		pass = false;
		goto done;
	}

	if (!piglit_check_gl_error(0)) {
		pass = false;
		goto done;
	}

	/* Issue #14 of the GL_ARB_separate_shader_objects spec says:
	 *
	 *     "14. Should glLinkProgram work to re-link a shader created with
	 *          glCreateShaderProgram?
	 *
	 *          RESOLVED: NO because the shader created by
	 *          glCreateShaderProgram is detached and deleted as part of
	 *          the glCreateShaderProgram sequence.  This means if you
	 *          call glLinkProgram on a program returned from
	 *          glCreateShaderProgram, you'll find the re-link fails
	 *          because no shader object is attached.
	 *
	 *          An application is free to attach one or more new shader
	 *          objects to the program and then relink would work.
	 *
	 *          This is fine because re-linking isn't necessary/expected."
	 */
	glLinkProgram(prog);

	if (piglit_is_core_profile) {
		if (piglit_link_check_status(prog)) {
			printf("Relinking program without any shaders "
			       "attached succeeded, but it should have "
			       "failed.\n");
			pass = false;
		}
	} else if (!piglit_link_check_status(prog)) {
		printf("Relinking program without any shaders attached "
		       "failed, but it should have succeeded.\n");
		pass = false;
	}

	pass = piglit_check_gl_error(0) && pass;

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, code);
	if (vs == 0) {
		pass = false;
		goto done;
	}

	glAttachShader(prog, vs);

	glLinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		printf("Relinking program after reattaching a vertex shader "
		       "failed, but it should have succeeded.\n");
		pass = false;
	}

	pass = piglit_check_gl_error(0) && pass;

 done:
	glDeleteProgram(prog);
	glDeleteShader(vs);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "relink a program created by "
				     "glCreateShaderProgramv");
	return pass;
}

static bool
glUseProgramStages_for_a_missing_stage(void)
{
	static const char *vs_code = "void main() { gl_Position = vec4(0); }";
	static const char *fs_code = "void main() { }";

	GLuint vs_prog = 0;
	GLuint fs_prog = 0;
	GLuint prog;
	GLuint pipe = 0;
	bool pass = true;

	vs_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					 (const GLchar * const*) &vs_code);
	fs_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
					 (const GLchar * const*) &fs_code);

	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);
	glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vs_prog);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, fs_prog);

	/* Sanity check.
	 */
	glGetProgramPipelineiv(pipe, GL_FRAGMENT_SHADER, (GLint *) &prog);
	if (prog != fs_prog) {
		printf("Sanity check failed - fragment shader program "
		       "mismatch.\n");
		pass = false;
	}

	glGetProgramPipelineiv(pipe, GL_VERTEX_SHADER, (GLint *) &prog);
	if (prog != vs_prog) {
		printf("Sanity check failed - vertex shader program "
		       "mismatch.\n");
		pass = false;
	}

	pass = piglit_check_gl_error(0) && pass;

	/* Issue #7 of the GL_ARB_separate_shader_objects spec says:
	 *
	 *     "7.  What happens if you have a program object current for a
	 *          shader stage, but the program object doesn't contain an
	 *          executable for that stage?
	 *
	 *          RESOLVED: This is not an error; instead it is as though
	 *          there were no program bound to that stage.  We have two
	 *          different notions for programs bound to shader stages.  A
	 *          program is "current" for a stage if it bound to that stage
	 *          in the active program pipeline object.  A program is
	 *          "active" for a stage if it is current and it has an
	 *          executable for this stage.  In this case, the program
	 *          would be current but not active.
	 *
	 *          When no program is active for a stage, the stage will be
	 *          replaced with fixed functionality logic (compatibility
	 *          profile vertex and fragment), disabled (tessellation
	 *          control and evaluation, geometry), or have undefined
	 *          results (core profile vertex and fragment).
	 *
	 *          Support for programs that are current but not active is
	 *          intentional behavior.  Consider an example where an
	 *          application wants to use two different types of separate
	 *          program object -- one for all types of vertex processing
	 *          and a second for fragment processing.  Some of the vertex
	 *          pipe programs might include tessellation or geometry
	 *          shaders; others might only include a vertex shader.  With
	 *          this configuration, the application can use code like the
	 *          following:
	 *
	 *            #define GL_ALL_VERTEX_PIPE_SHADER_BITS	\
	 *                (GL_VERTEX_SHADER_BIT             |	\
	 *                 GL_TESS_CONTROL_SHADER_BIT       |	\
	 *                 GL_TESS_EVALUATION_SHADER_BIT    |	\
	 *                 GL_GEOMETRY_SHADER_BIT)
	 *
	 *            glUseProgramStages(pipeline,
	 *                               GL_ALL_VERTEX_PIPE_SHADER_BITS,
	 *                               vertex_pipe_program);
	 *            glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT,
	 *                               fragment_pipe_program);
	 *
	 *        Such code wouldn't have to determine if
	 *        <vertex_pipe_program> has tessellation or geometry shaders.
	 *        Instead, it simply sets all possible bits, which removes the
	 *        old program from all non-fragment stages.  For stages not
	 *        present in the new program, the program will be current but
	 *        not active, and it will be as though no program were bound
	 *        to such stages."
	 *
	 * Further, the body of the spec says:
	 *
	 *     "If UseProgramStages is called with <program> set to zero or
	 *     with a program object that contains no executable code for a
	 *     given stages, it is as if the pipeline object has no
	 *     programmable stage configured for the indicated shader stages."
	 *
	 * This indicated to me that the "program == 0" and "program doesn't
	 * have the specified stage" cases should both cause
	 * glGetProgramPipelineiv to return zero for the GL_*_SHADER query.
	 */
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, vs_prog);

	glGetProgramPipelineiv(pipe, GL_FRAGMENT_SHADER, (GLint *) &prog);
	if (prog != 0) {
		printf("Using a program that lacks a particular stage for that "
		       "stage did not cause the stage to use program 0.\n");
		pass = false;
	}

	pass = piglit_check_gl_error(0) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "glUseProgramStages of a program that "
				     "lacks a specified stage");

	return pass;
}

static bool
glActiveShaderProgram_while_transform_feedback_is_active(void)
{
	static const char *vs_code = "void main() { gl_Position = vec4(0); }";
	static const char *fs_code = "void main() { }";

	GLuint vs_prog = 0;
	GLuint fs_prog = 0;
	GLuint pipe = 0;
	GLuint xfb = 0;
	GLuint buf = 0;
	bool pass = true;
	static const char *varyings[] = {"gl_Position"};

	if (!CreateShaderProgram_with_xfb(vs_code, varyings, 1, &vs_prog)) {
		pass = false;
		goto done;
	}

	fs_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
					 (const GLchar * const*) &fs_code);

	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);
	glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vs_prog);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, fs_prog);

	configure_transform_feedback_object(&xfb, &buf);

	pass = piglit_check_gl_error(0) && pass;

	glBeginTransformFeedback(GL_TRIANGLES);

	pass = piglit_check_gl_error(0) && pass;

	/* Issue #6b of the GL_ARB_separate_shader_objects spec says:
	 *
	 *     "6b. Should the active program be allowed to changed within
	 *          transform feedback mode?
	 *
	 *          RESOLVED:  Yes.
	 *
	 *          The active program simply allows uniforms to be changed
	 *          but doesn't actually change how the graphics pipeline
	 *          itself is configured or what programs are used for vertex,
	 *          geometry, and fragment processing."
	 */
	glActiveShaderProgram(pipe, vs_prog);
	glActiveShaderProgram(pipe, fs_prog);

	pass = piglit_check_gl_error(0) && pass;

	glEndTransformFeedback();

	pass = piglit_check_gl_error(0) && pass;

 done:
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	glDeleteTransformFeedbacks(1, &xfb);
	glDeleteBuffers(1, &buf);
	glDeleteProgram(vs_prog);
	glDeleteProgram(fs_prog);

	pass = piglit_check_gl_error(0) && pass;

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "glActiveShaderProgram while transform "
				     "feedback is active");

	return pass;
}

static bool
glBindProgramPipeline_while_transform_feedback_is_active(void)
{
	/* This is already covered by the "bind_pipeline" mode of the
	 * ext_transform_feedback-api-errors test.
	 */
	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_extension("GL_ARB_separate_shader_objects");

	pass = relink_program_created_by_glCreateShaderProgram() && pass;
	pass = glUseProgramStages_for_a_missing_stage() && pass;

	if (piglit_is_extension_supported("GL_ARB_transform_feedback2")) {
		pass = glActiveShaderProgram_while_transform_feedback_is_active()
			&& pass;
		pass = glBindProgramPipeline_while_transform_feedback_is_active()
			&& pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
