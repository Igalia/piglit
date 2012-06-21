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

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#if defined(HAVE_SYS_TIME_H) && defined(HAVE_SYS_RESOURCE_H) && defined(HAVE_SETRLIMIT)
#include <sys/time.h>
#include <sys/resource.h>
#define USE_SETRLIMIT
#endif

#if defined(HAVE_FCNTL_H) && defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H) && defined(HAVE_UNISTD_H)
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#else
# define USE_STDIO
#endif

#include "piglit-util.h"


#if defined(_WIN32)

/* Some versions of MinGW are missing _vscprintf's declaration, although they
 * still provide the symbol in the import library.
 */
#ifdef __MINGW32__
_CRTIMP int _vscprintf(const char *format, va_list argptr);
#endif

int asprintf(char **strp, const char *fmt, ...)
{
	va_list args;
	va_list args_copy;
	int length;
	size_t size;

	va_start(args, fmt);

	va_copy(args_copy, args);

#ifdef _WIN32
	/* We need to use _vcsprintf to calculate the length as vsnprintf returns -1
	 * if the number of characters to write is greater than count.
	 */
	length = _vscprintf(fmt, args_copy);
#else
	char dummy;
	length = vsnprintf(&dummy, sizeof dummy, fmt, args_copy);
#endif

	va_end(args_copy);

	assert(length >= 0);
	size = length + 1;

	*strp = malloc(size);
	if (!*strp) {
		return -1;
	}

	va_start(args, fmt);
	vsnprintf(*strp, size, fmt, args);
	va_end(args);

	return length;
}

#endif /* _WIN32 */

bool piglit_is_extension_in_string(const char *haystack, const char *needle)
{
	const unsigned needle_len = strlen(needle);

	if (needle_len == 0)
		return false;

	while (true) {
		const char *const s = strstr(haystack, needle);

		if (s == NULL)
			return false;

		if (s[needle_len] == ' ' || s[needle_len] == '\0') {
			return true;
		}

		/* strstr found an extension whose name begins with
		 * needle, but whose name is not equal to needle.
		 * Restart the search at s + needle_len so that we
		 * don't just find the same extension again and go
		 * into an infinite loop.
		 */
		haystack = s + needle_len;
	}

	return false;
}

/** Returns the line in the program string given the character position. */
int FindLine(const char *program, int position)
{
	int i, line = 1;
	for (i = 0; i < position; i++) {
		if (program[i] == '0')
			return -1; /* unknown line */
		if (program[i] == '\n')
			line++;
	}
	return line;
}

void
piglit_report_result(enum piglit_result result)
{
	fflush(stderr);

	if (result == PIGLIT_PASS) {
		printf("PIGLIT: {'result': 'pass' }\n");
		fflush(stdout);
		exit(0);
	} else if (result == PIGLIT_SKIP) {
		printf("PIGLIT: {'result': 'skip' }\n");
		fflush(stdout);
		exit(0);
	} else if (result == PIGLIT_WARN) {
		printf("PIGLIT: {'result': 'warn' }\n");
		fflush(stdout);
		exit(0);
	} else {
		printf("PIGLIT: {'result': 'fail' }\n");
		fflush(stdout);
		exit(1);
	}
}

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c)
{
	char *t = strchr(s, c);

	return (t == NULL) ? ((char *) s + strlen(s)) : t;
}
#endif


void
piglit_set_rlimit(unsigned long lim)
{
#if defined(USE_SETRLIMIT)
	struct rlimit rl;
	if (getrlimit(RLIMIT_AS, &rl) != -1) {
		printf("Address space limit = %lu, max = %lu\n",
		       (unsigned long) rl.rlim_cur,
		       (unsigned long) rl.rlim_max);

		if (rl.rlim_max > lim) {
			printf("Resetting limit to %lu.\n", lim);

			rl.rlim_cur = lim;
			rl.rlim_max = lim;
			if (setrlimit(RLIMIT_AS, &rl) == -1) {
				printf("Could not set rlimit "
				       "due to: %s (%d)\n",
				       strerror(errno), errno);
			}
		}
	}

	printf("\n");
#else
	printf("Cannot reset rlimit on this platform.\n\n");
#endif
}

/* Merges the PASS/FAIL/SKIP for @subtest into the overall result
 * @all.
 *
 * The @all should start out initialized to PIGLIT_SKIP.
 */
void
piglit_merge_result(enum piglit_result *all, enum piglit_result subtest)
{
	switch (subtest) {
	case PIGLIT_FAIL:
		*all = PIGLIT_FAIL;
		break;
	case PIGLIT_WARN:
		if (*all == PIGLIT_SKIP || *all == PIGLIT_PASS)
			*all = PIGLIT_WARN;
		break;
	case PIGLIT_PASS:
		if (*all == PIGLIT_SKIP)
			*all = PIGLIT_PASS;
		break;
	case PIGLIT_SKIP:
		break;
	}
}

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
