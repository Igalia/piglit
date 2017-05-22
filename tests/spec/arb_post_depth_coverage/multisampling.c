/*
 * Copyright (c) 2017 Red Hat Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"

/*
 * A test to check that when ARB_post_depth_coverage is enabled, the values in
 * gl_SampleMaskIn accurately reflect the results of the depth test being run
 * before the respective fragment shader invocation. As well, we also check to
 * make sure that when the extension is disabled, the values in
 * gl_SampleMaskIn do not reflect the results of the depth test in each
 * respective fragment shader invocation.
 * For good measure, we test this behavior at sample rates of 2, 4, 8, and 16
 * (if the GPU does not support a high enough sample rate to test all of these
 * rates, we skip the ones we can't test).
 */

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 43;
	config.supports_gl_core_version = 43;
	config.window_width = 160;
	config.window_height = 160;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH |
		PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

GLint prog1, prog2, prog3;

static const char *vs_text =
	"#version 430\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs_text1 =
	"#version 430\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	color = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"	gl_SampleMask[0] = 1;\n"
	"}\n";

static const char *fs_text2 =
	"#version 430\n"
	"#extension GL_ARB_post_depth_coverage: enable\n"
	"out vec4 color;\n"
	"layout(early_fragment_tests) in;\n"
	"layout(post_depth_coverage) in;\n"
	"layout(std430, binding = 0) buffer MaskOutput {\n"
	"	int data[];\n"
	"} mask_output;\n"
	"void main() {\n"
	"	int index = int(gl_FragCoord.y) * 160 + int(gl_FragCoord.x);\n"
	"	atomicOr(mask_output.data[index], gl_SampleMaskIn[0]);\n"
	"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

static const char *fs_text3 =
	"#version 430\n"
	"out vec4 color;\n"
	"layout(early_fragment_tests) in;\n"
	"layout(std430, binding = 0) buffer MaskOutput {\n"
	"	int data[];\n"
	"} mask_output;\n"
	"void main() {\n"
	"	int index = int(gl_FragCoord.y) * 160 + int(gl_FragCoord.x);\n"
	"	atomicOr(mask_output.data[index], gl_SampleMaskIn[0]);\n"
	"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

static inline bool
draw_and_check_sample_mask(GLint prog, int sample_count, int ssbo_value)
{
	const size_t sample_mask_size = piglit_width * piglit_height;
	GLint *sample_mask = calloc(sizeof(GLint), sample_mask_size);
	GLuint fbo, tex_color, tex_depth, ssbo;
	int i;
	bool ret = true;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* Create new textures */
	glGenTextures(1, &tex_color);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_color);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sample_count,
				GL_RGBA32F, piglit_width, piglit_height,
				false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE, tex_color, 0);

	glGenTextures(1, &tex_depth);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_depth);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sample_count,
				GL_DEPTH_COMPONENT24,
				piglit_width, piglit_height, false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			       GL_TEXTURE_2D_MULTISAMPLE, tex_depth, 0);

	/* Setup the ssbo */
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER,
		     sample_mask_size * sizeof(GLint),
		     sample_mask, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		ret = false;
		goto finish;
	}

	/* Draw a rectangle that covers the entire depth texture, but only in
	 * the first sample.
	 */
	glUseProgram(prog1);
	piglit_draw_rect_z(0.25, -1.0, -1.0, 4.0, 4.0);

	/* Now draw another rectangle that inhabits all of the samples, and
	 * see which ones are covered in gl_SampleMaskIn when the fragment
	 * shader is executed.
	 */
	glUseProgram(prog);
	piglit_draw_rect_z(0.5, -1.0, -1.0, 4.0, 4.0);

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
			   sample_mask_size * sizeof(GLint), sample_mask);
	for (i = 0; i < sample_mask_size; i++) {
		if (sample_mask[i] != ssbo_value) {
			fprintf(stderr,
				"(%d, %d) expected 0x%x in ssbo, got 0x%x\n",
				i % 160, i / 160, ssbo_value, sample_mask[i]);
			ret = false;
			break;
		}
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			  GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	piglit_present_results();

finish:
	glDeleteTextures(2, (GLuint[2]) { tex_color, tex_depth });
	glDeleteBuffers(1, &ssbo);
	glDeleteFramebuffers(1, &fbo);
	free(sample_mask);

	return ret;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_post_depth_coverage");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.2, 0.2, 0.2, 0.2);

	prog1 = piglit_build_simple_program(vs_text, fs_text1);
	prog2 = piglit_build_simple_program(vs_text, fs_text2);
	prog3 = piglit_build_simple_program(vs_text, fs_text3);
}

enum piglit_result
piglit_display(void)
{
	const int samples[] = {2, 4, 8, 16};
	int max_sample_count, mask, i, j;
	bool pass = true;

	glGetIntegerv(GL_MAX_SAMPLES, &max_sample_count);
	glViewport(0, 0, piglit_width, piglit_height);

	for (i = 0;
	     i < ARRAY_SIZE(samples) && samples[i] <= max_sample_count;
	     i++)
	{
		for (j = 0, mask = 0; j < samples[i]; j++)
			mask |= (1 << j);

		/* With post depth coverage, the depth test will be run on
		 * each sample before the fragment shader's invocation. As a
		 * result, sample 0 should fail the depth test and
		 * gl_SampleMaskIn[0] should indicate that all samples but 0
		 * are covered by the fragment shader.
		 */
		if (!draw_and_check_sample_mask(prog2, samples[i], mask & ~1))
			pass = false;

		/* Without post depth coverage, the depth test will not have
		 * been run by the time that the fragment shader is invoked,
		 * and thus gl_SampleMaskIn[0] will indicate that all samples
		 * are covered by the fragment shader.
		 */
		if (!draw_and_check_sample_mask(prog3, samples[i], mask))
			pass = false;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
