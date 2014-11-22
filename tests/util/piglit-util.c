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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#ifdef PIGLIT_HAS_POSIX_CLOCK_MONOTONIC
#ifdef PIGLIT_HAS_POSIX_TIMER_NOTIFY_THREAD
#include <pthread.h>
#include <signal.h>
#endif
#include <time.h>
#endif

#include "config.h"
#if defined(HAVE_SYS_TIME_H) && defined(HAVE_SYS_RESOURCE_H) && defined(HAVE_SETRLIMIT)
#include <sys/time.h>
#include <sys/resource.h>
#define USE_SETRLIMIT
#endif

#if defined(HAVE_FCNTL_H) && defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H) && defined(HAVE_UNISTD_H) && !defined(_WIN32)
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
#else
# define USE_STDIO
#endif

#include "piglit-util.h"


#ifndef HAVE_ASPRINTF

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

#endif /* HAVE_ASPRINTF */

/**
 * \brief Split \a string into an array of strings.
 *
 * The null-terminated string \a separators is a list of characters at
 * which to perform the splits. For example, if separators is " ,", then
 * the function will split the string at each occurence of ' ' and ','.
 */
const char**
piglit_split_string_to_array(const char *string, const char *separators)
{
	char **strings, *string_copy;
	int i, length, max_words;

	length = strlen(string);
	max_words = length / 2;
	strings = malloc((sizeof(char*) * (max_words + 1)) +
	                 (sizeof(char) * (length + 1)));
	assert(strings != NULL);

	string_copy = (char*) &strings[max_words + 1];
	strcpy(string_copy, string);

	strings[0] = strtok(string_copy, separators);
	for (i = 0; strings[i] != NULL; ++i) {
		strings[i + 1] = strtok(NULL, separators);
	}

	return (const char**) strings;
}

bool piglit_is_extension_in_array(const char **haystack, const char *needle)
{
	if (needle[0] == 0)
		return false;

	while (*haystack != NULL) {
		if (strcmp(*haystack, needle) == 0) {
			return true;
		}
		haystack++;
	}

	return false;
}

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
int piglit_find_line(const char *program, int position)
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

const char *
piglit_result_to_string(enum piglit_result result)
{
        switch (result) {
        case PIGLIT_FAIL: return "fail";
        case PIGLIT_SKIP: return "skip";
        case PIGLIT_WARN: return "warn";
        case PIGLIT_PASS: return "pass";
        }
        return "Unknown result";
}

void
piglit_report_result(enum piglit_result result)
{
	const char *result_str = piglit_result_to_string(result);

#ifdef PIGLIT_HAS_POSIX_TIMER_NOTIFY_THREAD
	/* Ensure we only report one result in case we race with timeout */
	static pthread_mutex_t result_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&result_lock);
#endif

	fflush(stderr);

	printf("PIGLIT: {\"result\": \"%s\" }\n", result_str);
	fflush(stdout);

	switch(result) {
	case PIGLIT_PASS:
	case PIGLIT_SKIP:
	case PIGLIT_WARN:
		exit(0);
	default:
		exit(1);
	}
}

#ifdef PIGLIT_HAS_POSIX_TIMER_NOTIFY_THREAD
static void
timeout_expired(union sigval val)
{
	piglit_loge("Test timed out.");
	piglit_report_result(val.sival_int);
}
#endif

void
piglit_set_timeout(double seconds, enum piglit_result timeout_result)
{
#ifdef PIGLIT_HAS_POSIX_TIMER_NOTIFY_THREAD
	struct sigevent sev = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = timeout_expired,
		.sigev_value = { .sival_int = timeout_result },
	};
	time_t sec = seconds;
	struct itimerspec spec = {
		.it_value = { .tv_sec = sec, .tv_nsec = (seconds - sec) * 1e9 },
	};
	timer_t timerid;
	timer_create(CLOCK_MONOTONIC, &sev, &timerid);
	timer_settime(timerid, 0, &spec, NULL);
#else
	piglit_logi("Cannot abort this test for timeout on this platform");
#endif
}

void
piglit_report_subtest_result(enum piglit_result result, const char *format, ...)
{
	const char *result_str = piglit_result_to_string(result);
	va_list ap;

	va_start(ap, format);

	printf("PIGLIT: {\"subtest\": {\"");
	vprintf(format, ap);
	printf("\" : \"%s\"}}\n", result_str);
	fflush(stdout);

	va_end(ap);
}

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c)
{
	char *t = strchr(s, c);

	return (t == NULL) ? ((char *) s + strlen(s)) : t;
}
#endif


#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
	const size_t len = strlen(s);
	const size_t size_to_copy = MIN2(n, len);

	char *const copy = malloc(size_to_copy + 1);
	if (copy != NULL) {
		memcpy(copy, s, size_to_copy);
		copy[size_to_copy] = '\0';
	}

	return copy;
}
#endif


#ifdef _WIN32

#ifndef DBG_PRINTEXCEPTION_C
#define DBG_PRINTEXCEPTION_C 0x40010006
#endif

static LONG WINAPI
exception_handler(PEXCEPTION_POINTERS pExceptionInfo)
{
	PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;

	/* Ignore OutputDebugStringA exceptions. */
	if (pExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	/* Ignore C++ exceptions
	 * http://support.microsoft.com/kb/185294
	 * http://blogs.msdn.com/b/oldnewthing/archive/2010/07/30/10044061.aspx
	 */
	if (pExceptionRecord->ExceptionCode == 0xe06d7363) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	/* Ignore thread naming exception.
	 * http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
	 */
	if (pExceptionRecord->ExceptionCode == 0x406d1388) {
		return EXCEPTION_CONTINUE_SEARCH;
	}

	fflush(stdout);
	fprintf(stderr, "error: uncaught exception 0x%08lx\n",
		pExceptionRecord->ExceptionCode);
	fflush(stderr);

	TerminateProcess(GetCurrentProcess(),
			 pExceptionRecord->ExceptionCode);

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif /* _WIN32 */


void
piglit_disable_error_message_boxes(void)
{
	/* When Windows' error message boxes are disabled for this process (as
	 * is always the case when running through `piglit run`) we disable CRT
	 * message boxes too.
	 *
	 * This will disable the CRT message boxes for the main executable, but
	 * it will not disable message boxes for assertion failures inside
	 * OpenGL ICD, unless this test's executable and the OpenGL ICD DLL are
	 * both dynamically linked to the same CRT DLL.  If the OpenGL ICD is
	 * statically linked to the CRT then it must do these calls itself.
	 */
#ifdef _WIN32
	UINT uMode;
#if _WIN32_WINNT >= 0x0600
	uMode = GetErrorMode();
#else
	uMode = SetErrorMode(0);
	SetErrorMode(uMode);
#endif
	if (uMode & SEM_FAILCRITICALERRORS) {
		/* Disable assertion failure message box.
		 * http://msdn.microsoft.com/en-us/library/sas1dkb2.aspx
		 */
		_set_error_mode(_OUT_TO_STDERR);
#ifdef _MSC_VER
		/* Disable abort message box.
		 * http://msdn.microsoft.com/en-us/library/e631wekh.aspx
		 */
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
	}

	/* Catch any exceptions and abort immediately.
	 *
	 * At least with certain implementations of GLUT (namely
	 * freeglut-2.8.1), the glutDisplayFunc()'s callback called inside a
	 * WindowProc callback function.
	 *
	 * And on certain cases (depending on the Windows version and 32 vs 64
	 * bits processes) uncaught exceptions inside WindowProc callbacks are
	 * silently ignored!  (See Remarks section of
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/ms633573.aspx
	 * page.)   The end result is that automatic tests end up blocking
	 * waiting for user input when an uncaught exceptionhappens, as control
	 * flow is interrupted before it reaches piglit_report_result(), and
	 * the process never aborts.
	 *
	 * By installing our own exception handler we can ensure that uncaught
	 * exceptions will never be silently ignored.
	 */
	AddVectoredExceptionHandler(0, exception_handler);
#endif /* _WIN32 */
}


void
piglit_set_rlimit(unsigned long lim)
{
#if defined(USE_SETRLIMIT) && defined(RLIMIT_AS)
	struct rlimit rl;
	if (getrlimit(RLIMIT_AS, &rl) != -1) {
		piglit_logi("Address space limit = %lu, max = %lu",
		       (unsigned long) rl.rlim_cur,
		       (unsigned long) rl.rlim_max);

		if (rl.rlim_max > lim) {
			piglit_logi("Resetting limit to %lu", lim);

			rl.rlim_cur = lim;
			rl.rlim_max = lim;
			if (setrlimit(RLIMIT_AS, &rl) == -1) {
				piglit_loge("Could not set rlimit "
				       "due to: %s (%d)",
				       strerror(errno), errno);
			}
		}
	}
#else
	piglit_loge("Cannot reset rlimit on this platform");
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

                if (!S_ISREG(st.st_mode) &&
                    !S_ISLNK(st.st_mode)) {
                   /* not a regular file or symlink */
                   close(fd);
                   return NULL;
                }

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

const char*
piglit_source_dir(void)
{

    const char *s = getenv("PIGLIT_SOURCE_DIR");

    if (s == NULL) {
        piglit_loge("env var PIGLIT_SOURCE_DIR is undefined");
        piglit_report_result(PIGLIT_FAIL);
    }

    return s;
}

#ifdef _WIN32
#  define PIGLIT_PATH_SEP '\\'
#else
#  define PIGLIT_PATH_SEP '/'
#endif

size_t
piglit_join_paths(char buf[], size_t buf_size, int n, ...)
{
	char *dest = buf;
	size_t size_written = 0;

	int i;
	va_list va;

	if (buf_size  == 0 || n < 1)
		return 0;

	va_start(va, n);

	i = 0;
	while (true) {
		const char *p = va_arg(va, const char*);

		while (*p != 0) {
			if (size_written == buf_size - 1)
				goto write_null;

			*dest = *p;
			++dest;
			++p;
			++size_written;
		}

		++i;
		if (i == n)
			break;

		*dest = PIGLIT_PATH_SEP;
		++dest;
		++size_written;
	}

write_null:
	*dest = '\0';
	++size_written;

	va_end(va);
	return size_written;
}

bool
piglit_time_is_monotonic(void)
{
#ifdef PIGLIT_HAS_POSIX_CLOCK_MONOTONIC
	struct timespec t;
	int r = clock_gettime(CLOCK_MONOTONIC, &t);

	return r == 0 || (r == -1 && errno != EINVAL);
#else
	return false;
#endif
}

int64_t
piglit_time_get_nano(void)
{
#if !defined(_WIN32)
#ifdef PIGLIT_HAS_POSIX_CLOCK_MONOTONIC
	struct timespec t;
	int r = clock_gettime(CLOCK_MONOTONIC, &t);

	if (r == 0 || (r == -1 && errno != EINVAL))
		return (t.tv_sec * INT64_C(1000000000)) + t.tv_nsec;
#endif
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_usec * INT64_C(1000) + tv.tv_sec * INT64_C(1000000000);
#else
	static LARGE_INTEGER frequency;
	LARGE_INTEGER counter;

	if (!frequency.QuadPart)
		QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * INT64_C(1000000000)/frequency.QuadPart;
#endif
}

/**
 * Search for an argument with the given name in the argument list.
 * If it is found, remove it and return true.
 */
bool
piglit_strip_arg(int *argc, char *argv[], const char *arg)
{
        int i;
        for (i = 1; i < *argc; i++) {
                if (strcmp(argv[i], arg) != 0)
                	continue;

                for (i += 1; i < *argc; ++i)
			argv[i-1] = argv[i];

		*argc -= 1;
		return true;
	}

        return false;
}

void
piglit_parse_subtest_args(int *argc, char *argv[],
			  const struct piglit_subtest *subtests,
			  const char ***out_selected_subtests,
			  size_t *out_num_selected_subtests)
{
	int j;
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;

	for (j = 1; j < *argc; j++) {
		if (streq(argv[j], "-subtest")) {
			int i;

			++j;
			if (j >= *argc) {
				piglit_loge("-subtest requires an argument");
				piglit_report_result(PIGLIT_FAIL);
			}

			if (!piglit_find_subtest(subtests, argv[j])) {
				piglit_loge("Test defines no subtest with "
					"name '%s'", argv[j]);
				piglit_report_result(PIGLIT_FAIL);
			}

			selected_subtests =
				realloc(selected_subtests,
					(num_selected_subtests + 1)
					* sizeof(char*));
			selected_subtests[num_selected_subtests] = argv[j];
			++num_selected_subtests;

			/* Remove 2 arguments from the command line. */
			for (i = j + 1; i < *argc; i++) {
				argv[i - 2] = argv[i];
			}
			*argc -= 2;
			j -= 2;
		} else if (streq(argv[j], "-list-subtests")) {
			int i;

			if (subtests == NULL) {
				piglit_loge("Test defines no subtests!");
				exit(EXIT_FAILURE);
			}

			for (i = 0; !PIGLIT_SUBTEST_END(&subtests[i]); ++i) {
				printf("%s: %s\n",
				       subtests[i].option,
				       subtests[i].name);
			}

			exit(EXIT_SUCCESS);
		}
	}

	*out_selected_subtests = selected_subtests;
	*out_num_selected_subtests = num_selected_subtests;
}


const struct piglit_subtest *
piglit_find_subtest(const struct piglit_subtest *subtests, const char *name)
{
	unsigned i;

	for (i = 0; !PIGLIT_SUBTEST_END(&subtests[i]); i++) {
		if (strcmp(subtests[i].option, name) == 0)
			return &subtests[i];
	}

	return NULL;
}

enum piglit_result
piglit_run_selected_subtests(const struct piglit_subtest *all_subtests,
			     const char **selected_subtests,
			     size_t num_selected_subtests,
			     enum piglit_result previous_result)
{
	enum piglit_result result = previous_result;

	if (num_selected_subtests) {
		unsigned i;

		for (i = 0; i < num_selected_subtests; i++) {
			enum piglit_result subtest_result;
			const char *const name = selected_subtests[i];
			const struct piglit_subtest *subtest =
				piglit_find_subtest(all_subtests, name);

			if (subtest == NULL) {
				piglit_loge("Unknown subtest \"%s\"", name);
				piglit_report_result(PIGLIT_FAIL);
			}

			subtest_result = subtest->subtest_func(subtest->data);
			piglit_report_subtest_result(subtest_result, "%s",
						     subtest->name);

			piglit_merge_result(&result, subtest_result);
		}
	} else {
		unsigned i;

		for (i = 0; !PIGLIT_SUBTEST_END(&all_subtests[i]); i++) {
			const enum piglit_result subtest_result =
				all_subtests[i].subtest_func(all_subtests[i].data);
			piglit_report_subtest_result(subtest_result, "%s",
						     all_subtests[i].name);

			piglit_merge_result(&result, subtest_result);
		}
	}

	return result;
}

uint64_t
piglit_gettid(void)
{
#ifdef __linux__
	return syscall(SYS_gettid);
#else
	return 0;
#endif
}
