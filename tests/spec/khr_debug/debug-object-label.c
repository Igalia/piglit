/*
 * Copyright (c) 2013 Timothy Arceri <t_arceri@yahoo.com.au>
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

static const char *TestLabel = "Test Label";
#define TestLabelLen 10

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 11;
#else /* using GLES */
	config.supports_gl_es_version = 20;
#endif

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#ifdef PIGLIT_USE_OPENGL
#define GET_FUNC(x) x
#else /* using GLES */
#define GET_FUNC(x) x ## KHR
#endif

/* These functions are assigned in the init function */
static PFNGLOBJECTPTRLABELPROC ObjectPtrLabel;
static PFNGLGETOBJECTPTRLABELPROC GetObjectPtrLabel;
static PFNGLOBJECTLABELPROC ObjectLabel;
static PFNGLGETOBJECTLABELPROC GetObjectLabel;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

static bool
test_object_ptr_label()
{
	GLsync sync;
	GLsizei length;
	GLchar label[TestLabelLen + 1];
	bool pass = true;

	puts("Test ObjectPtrLabel");

	/* basic check to see if glObjectPtrLabel/glGetObjectPtrLabel
	 * set/get the label
	 */
	sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	ObjectPtrLabel(sync, -1, TestLabel);
	GetObjectPtrLabel(sync, TestLabelLen + 1, &length, label);

	if (length != TestLabelLen || (strcmp(TestLabel, label) != 0)) {
		fprintf(stderr, "Label or length does not match\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		printf("  expected label: %s expected length: %i\n", TestLabel, TestLabelLen);
		pass = false;
	}
	glDeleteSync(sync);

	/* An INVALID_VALUE is generated if the <ptr> parameter of ObjectPtrLabel
	 * is not the name of a sync object.
	 */
	ObjectPtrLabel(NULL, length, label);

	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		fprintf(stderr, "GL_INVALID_VALUE should be generated when ObjectPtrLabel()"
			" ptr is not the name of a sync object\n");
		pass = false;
	}

	return pass;
}

/* <label> will be null-terminated. The actual number of
 * characters written into <label>,
 * excluding the null terminator, is returned in <length>.
 */
static bool
check_label_and_length(char *label, int length, char *object)
{
	bool pass = true;

	printf("Checking label and length of %s object\n", object);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	if (length != TestLabelLen || (strcmp(TestLabel, label) != 0)) {
		fprintf(stderr, "Label or length does not match in %s object\n", object);
		printf("  actual label: %s actual length: %i\n", label, length);
		printf("  expected label: %s expected length: %i\n", TestLabel, TestLabelLen);
		pass = false;
	}

	return pass;
}

/* trivial tests to get/set labels on all the different types of objects */
static bool
test_object_label_types()
{
	#define numObjects 12
	GLsizei length[numObjects]; /* create a fresh variable for each object to test */
	GLchar label[numObjects][TestLabelLen + 1];
	bool pass = true;

	enum test_object_indices {
		BUFFER_IDX, SHADER_IDX, PROGRAM_IDX, VERTEX_ARRAY_IDX, RENDERBUFFER_IDX,
		FRAMEBUFFER_IDX, QUERY_IDX, PROGRAM_PIPELINE_IDX, TRANSFORM_FEEDBACK_IDX,
		SAMPLER_IDX, TEXTURE_IDX, DISPLAY_LIST_IDX
	};

	GLuint buffer;
	GLuint shader;
	GLuint program;
	GLuint vertexArray;
	GLuint query;
#ifdef PIGLIT_USE_OPENGL
	GLuint programPipeline;
#endif
	GLuint transformFeedback;
	GLuint sampler;
	GLuint texture;
	GLuint renderbuffer;
	GLuint framebuffer;
#ifdef PIGLIT_USE_OPENGL
	GLuint displayList;
#endif

	/* Test BUFFER */
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	ObjectLabel(GL_BUFFER, buffer, -1, TestLabel);
	GetObjectLabel(GL_BUFFER, buffer, TestLabelLen + 1, &length[BUFFER_IDX], label[BUFFER_IDX]);

	check_label_and_length(label[BUFFER_IDX], length[BUFFER_IDX], "GL_BUFFER");

	glDeleteBuffers(1, &buffer);

	if (piglit_get_gl_version() >= 20) {
		/* Test SHADER */
		shader = glCreateShader(GL_FRAGMENT_SHADER);
		ObjectLabel(GL_SHADER, shader, -1, TestLabel);
		GetObjectLabel(GL_SHADER, shader, TestLabelLen + 1,
				 &length[SHADER_IDX], label[SHADER_IDX]);

		check_label_and_length(label[SHADER_IDX], length[SHADER_IDX], "GL_SHADER");

		glDeleteShader(shader);

		/* Test PROGRAM */
		program = glCreateProgram();
		ObjectLabel(GL_PROGRAM, program, -1, TestLabel);
		GetObjectLabel(GL_PROGRAM, program, TestLabelLen + 1,
				 &length[PROGRAM_IDX], label[PROGRAM_IDX]);

		check_label_and_length(label[PROGRAM_IDX], length[PROGRAM_IDX], "GL_PROGRAM");

		glDeleteProgram(program);
	}

	/* GL or GLES >= 3.0 supports vertex arrays*/
	if (piglit_get_gl_version() >= 30) {
		/* Test VERTEX_ARRAY */
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		ObjectLabel(GL_VERTEX_ARRAY, vertexArray, -1, TestLabel);
		GetObjectLabel(GL_VERTEX_ARRAY, vertexArray, TestLabelLen + 1,
				 &length[VERTEX_ARRAY_IDX], label[VERTEX_ARRAY_IDX]);

		check_label_and_length(label[VERTEX_ARRAY_IDX], length[VERTEX_ARRAY_IDX], "GL_VERTEX_ARRAY");

		glDeleteVertexArrays(1, &vertexArray);
	}

	/* GLES supports render buffer and frame buffer since 2.0 */
	if (piglit_is_gles() || piglit_get_gl_version() >= 30) {
		/* Test RENDERBUFFER */
		glGenRenderbuffers(1, &renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		ObjectLabel(GL_RENDERBUFFER, renderbuffer, -1, TestLabel);
		GetObjectLabel(GL_RENDERBUFFER, renderbuffer, TestLabelLen + 1,
				 &length[RENDERBUFFER_IDX], label[RENDERBUFFER_IDX]);

		check_label_and_length(label[RENDERBUFFER_IDX], length[RENDERBUFFER_IDX], "GL_RENDERBUFFER");

		glDeleteRenderbuffers(1, &renderbuffer);

		/* Test FRAMEBUFFER */
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		ObjectLabel(GL_FRAMEBUFFER, framebuffer, -1, TestLabel);
		GetObjectLabel(GL_FRAMEBUFFER, framebuffer, TestLabelLen + 1,
				 &length[FRAMEBUFFER_IDX], label[FRAMEBUFFER_IDX]);

		check_label_and_length(label[FRAMEBUFFER_IDX], length[FRAMEBUFFER_IDX], "GL_FRAMEBUFFER");

		glDeleteFramebuffers(1, &framebuffer);
	}

	/* GLES >= 3.0 or GL compat */
	if (!piglit_is_gles() || piglit_get_gl_version() >= 30) {
		/* Test QUERY */
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);
		glEndQuery(GL_TIME_ELAPSED);
		ObjectLabel(GL_QUERY, query, -1, TestLabel);
		GetObjectLabel(GL_QUERY, query, TestLabelLen + 1, &length[QUERY_IDX], label[QUERY_IDX]);

		check_label_and_length(label[QUERY_IDX], length[QUERY_IDX], "GL_TEST_QUERY");

		glDeleteQueries(1, &query);
	}

#ifdef PIGLIT_USE_OPENGL
	/* Test PROGRAM_PIPELINE */
	if (piglit_is_extension_supported("GL_ARB_separate_shader_objects")) {
		glGenProgramPipelines(1, &programPipeline);
		glBindProgramPipeline(programPipeline);
		ObjectLabel(GL_PROGRAM_PIPELINE, programPipeline, -1, TestLabel);
		GetObjectLabel(GL_PROGRAM_PIPELINE, programPipeline, TestLabelLen + 1,
				 &length[PROGRAM_PIPELINE_IDX], label[PROGRAM_PIPELINE_IDX]);

		check_label_and_length(label[PROGRAM_PIPELINE_IDX], length[PROGRAM_PIPELINE_IDX], "GL_PROGRAM_PIPELINE");

		glDeleteProgramPipelines(1, &programPipeline);
	}
#endif /* PIGLIT_USE_OPENGL */

	/* Test TRANSFORM_FEEDBACK */
	if ((piglit_is_gles() && piglit_get_gl_version() >= 30) ||
		piglit_is_extension_supported("GL_ARB_transform_feedback2")) {
		glGenTransformFeedbacks(1, &transformFeedback);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback);
		ObjectLabel(GL_TRANSFORM_FEEDBACK, transformFeedback, -1, TestLabel);
		GetObjectLabel(GL_TRANSFORM_FEEDBACK, transformFeedback, TestLabelLen + 1,
				 &length[TRANSFORM_FEEDBACK_IDX], label[TRANSFORM_FEEDBACK_IDX]);

		check_label_and_length(label[TRANSFORM_FEEDBACK_IDX], length[TRANSFORM_FEEDBACK_IDX], "GL_TRANSFORM_FEEDBACK");

		glDeleteTransformFeedbacks(1, &transformFeedback);
	}

	/* Test SAMPLER */
	if ((piglit_is_gles() && piglit_get_gl_version() >= 30) ||
		piglit_is_extension_supported("GL_ARB_sampler_objects")) {
		glGenSamplers(1, &sampler);
		glBindSampler(0, sampler);
		ObjectLabel(GL_SAMPLER, sampler, -1, TestLabel);
		GetObjectLabel(GL_SAMPLER, sampler, TestLabelLen + 1, &length[SAMPLER_IDX], label[SAMPLER_IDX]);

		check_label_and_length(label[SAMPLER_IDX], length[SAMPLER_IDX], "GL_SAMPLER");

		glDeleteSamplers(1, &sampler);
	}

	/* Test TEXTURE */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	ObjectLabel(GL_TEXTURE, texture, -1, TestLabel);
	GetObjectLabel(GL_TEXTURE, texture, TestLabelLen + 1, &length[TEXTURE_IDX], label[TEXTURE_IDX]);

	check_label_and_length(label[TEXTURE_IDX], length[TEXTURE_IDX], "GL_TEXTURE");

	glDeleteTextures(1, &texture);

#ifdef PIGLIT_USE_OPENGL
	/* Test DISPLAY_LIST - Compatibility Profile */
	displayList = glGenLists(1);
	glNewList(displayList, GL_COMPILE_AND_EXECUTE);
	glEndList();
	ObjectLabel(GL_DISPLAY_LIST, displayList, -1, TestLabel);
	GetObjectLabel(GL_DISPLAY_LIST, displayList, TestLabelLen + 1, &length[DISPLAY_LIST_IDX], label[DISPLAY_LIST_IDX]);

	check_label_and_length(label[DISPLAY_LIST_IDX], length[DISPLAY_LIST_IDX], "GL_DISPLAY_LIST");

	glDeleteLists(displayList, 1);
#endif /* PIGLIT_USE_OPENGL */

	return pass;
}

static bool
test_object_label()
{
	GLsizei length;
	GLuint buffer;
	GLuint invalidBufferName;
	GLint maxLabelLength;
	GLchar label[TestLabelLen + 1];
	GLchar *bigLabel;
	bool pass = true;
	int maximumLabelLengthTest = 1024; /* Be defensive about the size label length test to avoid memory issues */

	puts("Test ObjectLabel");

	glGenBuffers(1, &buffer);

	/* An INVALID_VALUE error is generated if the number of characters in
	 * <label>, excluding the null terminator when <length> is negative, is not
	 * less than the value of MAX_LABEL_LENGTH.
	 */
	glGetIntegerv(GL_MAX_LABEL_LENGTH, &maxLabelLength);
	if (maxLabelLength <= maximumLabelLengthTest) {
		bigLabel = (char *) malloc(maxLabelLength + 1);
		if (bigLabel){
			memset(bigLabel,'a',maxLabelLength);
			bigLabel[maxLabelLength] = '\0';

			/* Test when length -1 */
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			ObjectLabel(GL_BUFFER, buffer, -1, bigLabel);

			if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
				fprintf(stderr, "GL_INVALID_VALUE should be generated when label >= MAX_LABEL_LENGTH\n");
				pass = false;
			}

			/* test with large client defined length */
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			ObjectLabel(GL_BUFFER, buffer, maxLabelLength, bigLabel);

			if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
				fprintf(stderr, "GL_INVALID_VALUE should be generated when label length >= MAX_LABEL_LENGTH\n");
				pass = false;
			}
		}
	}
	else {
		printf("MAX_LABEL_LENGTH test skipped as implementations MAX_LABEL_LENGTH=%i and max piglit test length=%i\n",
		       maxLabelLength, maximumLabelLengthTest);
	}

	/* If <label> is NULL, any debug label is effectively removed from the object.
	 */
	ObjectLabel(GL_BUFFER, buffer, -1, TestLabel);
	ObjectLabel(GL_BUFFER, buffer, -1, NULL);
	GetObjectLabel(GL_BUFFER, buffer, TestLabelLen + 1, &length, label);

	if (length != 0 || (strcmp("", label) != 0)) {
		fprintf(stderr, "Setting label to NULL should remove the label\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		pass = false;
	}

	/* An INVALID_ENUM error is generated by ObjectLabel if <identifier> is not
	 * one of the object types.
	 */
	ObjectLabel(GL_ARRAY_BUFFER, buffer, -1, TestLabel);

	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		fprintf(stderr, "GL_INVALID_ENUM should be generated when the ObjectLabel identifier is invalid\n");
		pass = false;
	}

	/* An INVALID_VALUE error is generated by ObjectLabel if <name> is not
	 * the name of a valid object of the type specified by <identifier>.
	 */
	invalidBufferName = buffer;
	glDeleteBuffers(1, &buffer);
	ObjectLabel(GL_BUFFER, invalidBufferName, -1, TestLabel);

	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		fprintf(stderr, "GL_INVALID_VALUE should be generated when the ObjectLabel name is invalid\n");
		pass = false;
	}

	return pass;
}

static bool
test_get_object_label()
{
	#define numBuffers 4
	GLsizei length;
	GLuint buffers[numBuffers];
	GLuint invalidBufferName;
	GLchar label[TestLabelLen + 1];
	bool pass = true;

	enum test_object_indices {
		TEST_BUFSIZE_IDX, TEST_NO_LABEL_IDX, TEST_NULL_LABEL_IDX, TEST_NULL_LENGTH_IDX
	};

	puts("Test GetObjectLabel");

	glGenBuffers(numBuffers, buffers);

	/* The maximum number of characters that may
	 * be written into <label>, including the null terminator, is specified by
	 * <bufSize>.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_BUFSIZE_IDX]);
	ObjectLabel(GL_BUFFER, buffers[TEST_BUFSIZE_IDX], -1, TestLabel);
	GetObjectLabel(GL_BUFFER, buffers[TEST_BUFSIZE_IDX], TestLabelLen, &length, label);

	if (length != 9 || (strcmp("Test Labe", label) != 0)) {
		fprintf(stderr, "BufSize should limit the maximum label length to 9\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		pass = false;
	}

	/* If no debug label was specified for the object then <label>
	 * will contain a null-terminated empty string, and zero will be returned
	 * in <length>.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_NO_LABEL_IDX]);
	GetObjectLabel(GL_BUFFER, buffers[TEST_NO_LABEL_IDX], TestLabelLen + 1, &length, label);

	if (length != 0 || (strcmp("", label) != 0)) {
		fprintf(stderr, "Label should be empty and length 0\n");
		printf("  actual label: %s actual length: %i\n", label, length);
		pass = false;
	}

	/* If <label> is NULL and <length> is non-NULL then no string
	 * will be returned and the length of the label will be returned in
	 * <length>.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_NULL_LABEL_IDX]);
	ObjectLabel(GL_BUFFER, buffers[TEST_NULL_LABEL_IDX], -1, TestLabel);
	GetObjectLabel(GL_BUFFER, buffers[TEST_NULL_LABEL_IDX], TestLabelLen + 1, &length, NULL);

	if (length != TestLabelLen) {
		fprintf(stderr, "Label length should be %i\n", TestLabelLen);
		printf("  actual length: %i\n", length);
		pass = false;
	}

	/* If <length> is NULL, no length is returned.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TEST_NULL_LENGTH_IDX]);
	ObjectLabel(GL_BUFFER, buffers[TEST_NULL_LENGTH_IDX], -1, TestLabel);
	GetObjectLabel(GL_BUFFER, buffers[TEST_NULL_LENGTH_IDX], TestLabelLen + 1, NULL, label);

	if (strcmp(TestLabel, label) != 0) {
		fprintf(stderr, "Label doent match expected string when length NULL\n");
		printf("  label: %s expected: %s\n", label, TestLabel);
		pass = false;
	}

	/* An INVALID_ENUM error is generated by GetObjectLabel if identifier is not
	 * one of the valid object types
	 */
	GetObjectLabel(GL_ARRAY_BUFFER, buffers[TEST_NULL_LENGTH_IDX], TestLabelLen + 1, &length, label);

	if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
		fprintf(stderr, "GL_INVALID_ENUM should be generated when GetObjectLabel identifier is invalid\n");
		pass = false;
	}

	/* An INVALID_VALUE error is generated by GetObjectLabel if <name> is not
	 * the name of a valid object of the type specified by <identifier>.
	 */
	invalidBufferName = buffers[TEST_NULL_LENGTH_IDX];
	glDeleteBuffers(numBuffers, buffers);
	GetObjectLabel(GL_BUFFER, invalidBufferName, TestLabelLen + 1, &length, label);

	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		fprintf(stderr, "GL_INVALID_VALUE should be generated when GetObjectLabel name is invalid\n");
		pass = false;
	}

	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

	ObjectPtrLabel = GET_FUNC(glObjectPtrLabel);
	GetObjectPtrLabel = GET_FUNC(glGetObjectPtrLabel);
	ObjectLabel = GET_FUNC(glObjectLabel);
	GetObjectLabel = GET_FUNC(glGetObjectLabel);

#ifdef PIGLIT_USE_OPENGL
	piglit_require_gl_version(15);
#endif
	piglit_require_extension("GL_KHR_debug");

	pass = test_object_label_types() && pass;
	pass = test_object_label() && pass;
	pass = test_get_object_label() && pass;

	/* Test only if is GLES 3.0 or GL 3.2 or has ARB_sync */
	if ((piglit_is_gles() && piglit_get_gl_version() >= 30) ||
	    piglit_get_gl_version() >= 32 ||
	    piglit_is_extension_supported("GL_ARB_sync"))
		pass = test_object_ptr_label() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
