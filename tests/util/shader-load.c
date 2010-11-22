/*
 * Copyright © 2007, 2008 Ian D. Romanick
 * All Rights Reserved.
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
 * AUTHORS, COPYRIGHT HOLDERS, AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "config.h"

#if defined(HAVE_FCNTL_H) && defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H) && defined(HAVE_UNISTD_H)
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#else
# define USE_STDIO

# ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif
#endif

#include <stdio.h>
#include <stdlib.h>


char *piglit_load_text_file(const char *file_name, unsigned *size)
{
	char *text = NULL;

#if defined(USE_STDIO)
	FILE *fp;

# ifdef HAVE_FOPEN_S
	errno_t err;

	if (file_name == NULL) {
		return NULL;
	}

	err = fopen_s(&fp, file_name, "r");

	if (err || (fp == NULL)) {
		return NULL;
	}
# else
	fp = fopen(file_name, "r");
	if (fp == NULL) {
		return NULL;
	}
# endif

	if (fseek(fp, 0, SEEK_END) == 0) {
		size_t len = (size_t) ftell(fp);
		rewind(fp);

		text = malloc(len + 1);
		if (text != NULL) {
			size_t total_read = 0;

			do {
				size_t bytes = fread(text + total_read, 1,
						     len - total_read, fp);

				total_read += bytes;
				if (feof(fp)) {
					break;
				}

				if (ferror(fp)) {
					free(text);
					text = NULL;
					break;
				}
			} while (total_read < len);

			if (text != NULL) {
				text[total_read] = '\0';
			}

			if (size != NULL) {
				*size = total_read;
			}
		}
	}

	fclose(fp);
	return text;
#else
	struct stat st;
	int fd = open(file_name, O_RDONLY);

	if (fd < 0) {
		return NULL;
	}

	if (fstat(fd, & st) == 0) {
		ssize_t total_read = 0;

		text = malloc(st.st_size + 1);
		if (text != NULL) {
			do {
				ssize_t bytes = read(fd, text + total_read,
						     st.st_size - total_read);
				if (bytes < 0) {
					free(text);
					text = NULL;
					break;
				}

				if (bytes == 0) {
					break;
				}

				total_read += bytes;
			} while (total_read < st.st_size);

			text[total_read] = '\0';
			if (size != NULL) {
				*size = total_read;
			}
		}
	}

	close(fd);

	return text;
#endif
}
