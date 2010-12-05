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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <ctype.h>

#include "piglit-util.h"

#ifndef TRUE
#define FALSE   0
#define TRUE    (!FALSE)
#endif

int automatic = FALSE;

char *
unix_line_endings(const char *input, size_t length)
{
	char *output = malloc(length + 1);
	unsigned i;
	unsigned j = 0;

	for (i = 0; i < length; i++) {
		if ((input[i] == 0x0D) && (input[i + 1] == 0x0A)) {
			i++;
			output[j] = 0x0A;
		} else {
			output[j] = input[i];
		}

		j++;
	}

	output[j] = '\0';
	return output;
}


char *
dos_line_endings(const char *input, size_t length)
{
	char *output;
	unsigned i;
	unsigned j = 0;
	size_t new_length = length;


	for (i = 0; i < length; i++) {
		if (input[i] == 0x0A) {
			new_length++;
		}
	}

	output = malloc(new_length + 1);
	for (i = 0; i < length; i++) {
		if ((input[i] == 0x0D) && (input[i + 1] == 0x0A)) {
			i++;
			output[j + 0] = 0x0D;
			output[j + 1] = 0x0A;
			j += 2;
		} else if (input[i] == 0x0A) {
			output[j + 0] = 0x0D;
			output[j + 1] = 0x0A;
			j += 2;
		} else {
			output[j] = input[i];
			j++;
		}
	}

	output[j] = '\0';
	return output;
}


void
compile(const char *filename, GLenum target, int use_ARB)
{
	GLenum err;
	GLuint prognum[2];
	char *buf;
	char *ptr;
	unsigned sz;
	int expected_fail;
	char *converted_buffers[2];
	size_t buffer_sizes[2];
	unsigned i;


	if (!automatic) {
		printf("%s:\n", filename);
	}

	buf = piglit_load_text_file(filename, &sz);
	if (buf == NULL) {
		piglit_report_result(PIGLIT_FAILURE);
	}


	/* Scan the program source looking for two different things.  First,
	 * look for comments of the form '# FAIL'.  This signals that the
	 * program is expected to fail compilation.  Second, look for comments
	 * of the form '# REQUIRE GL_XXX_xxxx_xxxx'.  This signals that the
	 * program will only compile if some OpenGL extension is available.
	 */
	expected_fail = (strstr(buf, "# FAIL") != NULL);

	ptr = buf;
	while (ptr != NULL) {
		ptr = strstr(ptr, "# REQUIRE ");
		if (ptr != NULL) {
			char extension[128];
			unsigned i;

			ptr += strlen("# REQUIRE ");

			for (i = 0; !isspace(ptr[i]) && (ptr[i] != '\0'); i++) {
				extension[i] = ptr[i];
			}

			extension[i] = '\0';

			if (!glutExtensionSupported(extension)) {
				printf("Test requires %s\n", extension);
				piglit_report_result(PIGLIT_SKIP);

				free(buf);
				return;
			}
		}
	}


	converted_buffers[0] = unix_line_endings(buf, sz);
	buffer_sizes[0] = strlen(converted_buffers[0]);
	converted_buffers[1] = dos_line_endings(buf, sz);
	buffer_sizes[1] = strlen(converted_buffers[1]);

	if (use_ARB) {
		glEnable(target);
		glGenProgramsARB(2, prognum);
	} else {
		glGenProgramsNV(2, prognum);
	}


	for (i = 0; i < 2; i++) {
		/* The use_ARB flag is used instead of the target because
		 * GL_VERTEX_PROGRAM_ARB and GL_VERTEX_PROGRAM_NV have the same
		 * value.
		 */
		if (use_ARB) {
			glBindProgramARB(target, prognum[i]);
			glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB,
					   buffer_sizes[i],
					   (const GLubyte *) converted_buffers[i]);
		} else {
			glBindProgramNV(target, prognum[i]);
			glLoadProgramNV(target, prognum[i],
					buffer_sizes[i],
					(const GLubyte *) converted_buffers[i]);
		}


		err = glGetError();
		if (err != GL_NO_ERROR) {
			GLint errorpos;

			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorpos);
			if (!automatic) {
				printf("glGetError = 0x%04x\n", err);
				printf("errorpos: %d\n", errorpos);
				printf("%s\n",
				       (char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));
			}
		}

		if ((err == GL_NO_ERROR) != (expected_fail == FALSE)) {
			piglit_report_result(PIGLIT_FAILURE);
		}
	}

	free(buf);
	free(converted_buffers[0]);
	free(converted_buffers[1]);

}


int
main(int argc, char **argv)
{
	GLenum target;
	unsigned i;
	int use_ARB;


	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(250, 250);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
	glutCreateWindow("assembler test");
	glewInit();

	i = 1;
	if (((argc - 1) >= 1) && (strcmp(argv[i], "-auto") == 0)) {
		automatic = TRUE;
		i++;
	}

	if ((argc - i) < 2) {
		piglit_report_result(PIGLIT_FAILURE);
	}


	use_ARB = 1;
	if (strcmp(argv[i], "ARBvp1.0") == 0) {
		target = GL_VERTEX_PROGRAM_ARB;
		piglit_require_extension("GL_ARB_vertex_program");
	} else if (strcmp(argv[i], "ARBfp1.0") == 0) {
		target = GL_FRAGMENT_PROGRAM_ARB;
		piglit_require_extension("GL_ARB_fragment_program");
	} else if (strcmp(argv[i], "NVvp1.0") == 0) {
		target = GL_VERTEX_PROGRAM_NV;
		piglit_require_extension("GL_NV_vertex_program");
		use_ARB = 0;
	} else if (strcmp(argv[i], "NVfp1.0") == 0) {
		target = GL_FRAGMENT_PROGRAM_NV;
		piglit_require_extension("GL_NV_fragment_program");
		use_ARB = 0;
	} else {
		piglit_report_result(PIGLIT_FAILURE);
	}

	for (i++; i < argc; i++) {
		compile(argv[i], target, use_ARB);
	}

	piglit_report_result(PIGLIT_SUCCESS);
	return 0;
}
