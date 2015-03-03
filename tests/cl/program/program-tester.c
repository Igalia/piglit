/*
 * Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
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

/**
 * @file program_tester.c
 *
 * Parser and runner for building programs and executing kernels.
 */

#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <regex.h>
#include <libgen.h>

#include "piglit-framework-cl-program.h"

/* Regexes */

/*
 * IMPORTANT:
 *     Watch on subexpressions when modifying regexes, because some of them are
 *     used to retrieve the strings matching them!
 */

/*
 * Section (section can include whitespace):
 *   <whitespace>[<whitespace>section<whitespace>]<whitespace>
 */
#define REGEX_SECTION "^[[:space:]]*\\[[[:space:]]*([[:alnum:]_]+[[:alnum:][:space:]_]*[[:alnum:]_]+|[[:alnum:]_]+)[[:space:]]*\\][[:space:]]*$" /* section */
/*
 * Key-value (value can have whitespace):
 *   <whitespace>key<whitespace>:<whitespace>value<whitespace>
 */
#define REGEX_KEY_VALUE "^[[:space:]]*([[:alnum:]_]+)[[:space:]]*" /* key */ \
                         ":" \
                         "[[:space:]]*([^[:space:]#]+[^#]*[^[:space:]#]+|[^[:space:]#]+)[[:space:]]*$" /* value */
/*
 * Ignored:
 *   <whitespace>
 */
#define REGEX_IGNORE "^[[:space:]]*$"

/* Values */
#define REGEX_ARRAY_DELIMITER        "[[:space:]]+"
#define REGEX_DEFINE_ARRAY(element)  "((" element REGEX_ARRAY_DELIMITER ")*" element ")"
#define REGEX_NAN          "(nan|NAN|NaN)"
#define REGEX_PNAN         "([+]?" REGEX_NAN ")"
#define REGEX_NNAN         "([-]" REGEX_NAN ")"
#define REGEX_INF          "(infinity|INFINITY|Infinity|inf|INF|Inf)"
#define REGEX_PINF         "([+]?" REGEX_INF ")"
#define REGEX_NINF         "([-]" REGEX_INF ")"
#define REGEX_NULL         "(NULL|null)"
#define REGEX_BOOL         "(0|1|false|true)"
#define REGEX_BOOL_TRUE    "(1|true)"
#define REGEX_BOOL_FALSE   "(0|false)"
#define REGEX_INT          "(([+-]?[[:digit:]]+)|([+-]?0[Xx][[:digit:]abcdefABCDEF]+))"
#define REGEX_UINT         "(([+]?[[:digit:]]+)|([+]?0[Xx][[:digit:]abcdefABCDEF]+))"
#define REGEX_FLOAT_HEX    "([+-]?0[Xx][[:digit:]abcdefABCDEF.]+[[:digit:]pP+-]*)"
#define REGEX_FLOAT        "(([+-]?[[:digit:]]+(\\.[[:digit:]]+)?e*[+-]*[[:digit:]]*)|"      \
                             REGEX_FLOAT_HEX "|" REGEX_PNAN "|" REGEX_NNAN "|" REGEX_PINF "|" \
                             REGEX_NINF ")"
#define REGEX_BOOL_ARRAY   REGEX_DEFINE_ARRAY(REGEX_BOOL)
#define REGEX_INT_ARRAY    REGEX_DEFINE_ARRAY(REGEX_INT)
#define REGEX_UINT_ARRAY   REGEX_DEFINE_ARRAY(REGEX_UINT)
#define REGEX_FLOAT_ARRAY  REGEX_DEFINE_ARRAY(REGEX_FLOAT)
#define REGEX_VALUE        "(" REGEX_NULL "|" REGEX_BOOL "|" REGEX_INT "|" \
                               REGEX_UINT "|" REGEX_FLOAT ")"
#define REGEX_ARRAY        "(" REGEX_NULL "|" REGEX_BOOL_ARRAY "|"      \
                               REGEX_INT_ARRAY "|" REGEX_UINT_ARRAY "|" \
                               REGEX_FLOAT_ARRAY ")"
#define REGEX_ARRAY_VALUE  "(" REGEX_BOOL "|" REGEX_INT "|" REGEX_UINT "|" \
                               REGEX_FLOAT ")"
#define REGEX_RANDOM       "(RANDOM|random)"
#define REGEX_REPEAT       "(REPEAT|repeat)[[:space:]]+" \
                           REGEX_DEFINE_ARRAY(REGEX_ARRAY_VALUE)

/* Types */
#define REGEX_DEFINE_TYPE(type)  type"|"type"2|"type"3|"type"4|"type"8|"type"16"
#define REGEX_TYPE_CHAR       REGEX_DEFINE_TYPE("char")
#define REGEX_TYPE_UCHAR      REGEX_DEFINE_TYPE("uchar")
#define REGEX_TYPE_SHORT      REGEX_DEFINE_TYPE("short")
#define REGEX_TYPE_USHORT     REGEX_DEFINE_TYPE("ushort")
#define REGEX_TYPE_INT        REGEX_DEFINE_TYPE("int")
#define REGEX_TYPE_UINT       REGEX_DEFINE_TYPE("uint")
#define REGEX_TYPE_LONG       REGEX_DEFINE_TYPE("long")
#define REGEX_TYPE_ULONG      REGEX_DEFINE_TYPE("ulong")
// half is defined as unsigned short and C can't read/write its value.
// Also half is only used as a storage format unless device supports
// cl_khr_fp16
// TODO: probably we could use libmpdec to handle this
//       http://www.bytereef.org/mpdecimal/index.html
//#define REGEX_TYPE_HALF       "buffer half[1]"
#define REGEX_TYPE_FLOAT      REGEX_DEFINE_TYPE("float")
#define REGEX_TYPE_DOUBLE      REGEX_DEFINE_TYPE("double")
#define REGEX_TYPE  REGEX_TYPE_CHAR "|" REGEX_TYPE_UCHAR "|" \
                    REGEX_TYPE_SHORT "|" REGEX_TYPE_USHORT "|" \
                    REGEX_TYPE_INT "|" REGEX_TYPE_UINT "|" REGEX_TYPE_LONG "|" \
                    REGEX_TYPE_ULONG "|" REGEX_TYPE_FLOAT "|" REGEX_TYPE_DOUBLE

/*
 * Value argument:
 *   index<whitespace>type<whitespace>value
 * Buffer argument:
 *   index<whitespace>buffer<whitespace>type[size]<whitespace>(value|random|repeat value)<whitespace>tolerance<whitespace>value
 */
#define REGEX_DEFINE_ARG(type, value)  "([[:digit:]]+)[[:space:]]+" type \
                                       "[[:space:]]+(" value ")"
#define REGEX_ARG_TOLERANCE            "tolerance[[:space:]]+(" REGEX_VALUE ")"
#define REGEX_ARG_TOLERANCE_ULP        REGEX_ARG_TOLERANCE "[[:space:]]+ulp"
#define REGEX_ARG_VALUE   REGEX_DEFINE_ARG( "(" REGEX_TYPE ")", REGEX_ARRAY )
#define REGEX_ARG_BUFFER  REGEX_DEFINE_ARG( "buffer[[:space:]]+(" REGEX_TYPE ")\\[([[:digit:]]+)\\]", \
                                            REGEX_ARRAY "|" REGEX_RANDOM "|" REGEX_REPEAT )           \
                          "([[:space:]]+" "("REGEX_ARG_TOLERANCE "|" REGEX_ARG_TOLERANCE_ULP")" ")?"
#define REGEX_ARG         "(" REGEX_ARG_VALUE "|" REGEX_ARG_BUFFER ")"

/* Match whole line */
#define REGEX_FULL_MATCH(content) "^"content"$"

/* Comment config */
#define REGEX_COMMENT_CONFIG "/\\*!(.*)!\\*/"

/* Other */
#define REGEX_LINE       "^([^#]*)(#.*)?$"
#define REGEX_MULTILINE  "^([^#]*)\\\\[[:space:]]*$"

/* Config function */
void init(const int argc,
          const char** argv,
          struct piglit_cl_program_test_config* config);
void clean(const int argc,
           const char** argv,
           const struct piglit_cl_program_test_config* config);


/* Kernel test configuration */

// Framework options
PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN

	config.init_func = (piglit_cl_test_init_t*)init;
	config.clean_func = (piglit_cl_test_clean_t*)clean;

PIGLIT_CL_PROGRAM_TEST_CONFIG_END

// Additional options
bool     expect_test_fail = false;
cl_uint  work_dimensions = 1;
size_t   global_work_size[3] = {1, 1, 1};
size_t   local_work_size[3] = {1, 1, 1};
bool     local_work_size_null = false;

/* Helper functions */

void
add_dynamic_array(void** array,
                  unsigned int* count,
                  size_t element_size,
                  void* data)
{
#define GROW_SIZE 8
	if(((*count)%GROW_SIZE) == 0) {
		void* old_array = *array;

		*array = malloc(((*count)/GROW_SIZE + 1)*GROW_SIZE * element_size);
		memcpy(*array, old_array, (*count) * element_size);

		free(old_array);
	}

	memcpy((char *)(*array) + ((*count)*element_size), data, element_size);
	(*count)++;
#undef GROW_SIZE
}

/* Tests */

enum test_arg_type {
	TEST_ARG_VALUE,
	TEST_ARG_BUFFER,
};

enum cl_type {
	TYPE_CHAR,
	TYPE_UCHAR,
	TYPE_SHORT,
	TYPE_USHORT,
	TYPE_INT,
	TYPE_UINT,
	TYPE_LONG,
	TYPE_ULONG,
	TYPE_FLOAT,
	TYPE_DOUBLE,
};

struct test_arg {
	enum test_arg_type type;

	enum cl_type cl_type;
	size_t cl_size; // 1 for int, 3 for int3
	size_t cl_mem_size; // 1 for int, 4 for int3
	size_t length; // for buffers

	/* kernel arg data */
	cl_uint index;
	size_t size;
	void* value;

	/* tolerance */
	int64_t toli;
	uint64_t tolu;
	uint64_t ulp;
};

struct test_arg create_test_arg()
{
	struct test_arg ta = {
		.type = TEST_ARG_VALUE,

		.cl_type = TYPE_CHAR,
		.cl_size = 1,
		.cl_mem_size = 1,
		.length = 0,

		.index = 0,
		.size = 0,
		.value = NULL,

		.toli = 0,
		.tolu = 0,
		.ulp = 0,
	};

	return ta;
}

struct test {
	char* name;

	char* kernel_name;
	cl_uint work_dimensions;
	size_t global_work_size[3];
	size_t local_work_size[3];
	bool local_work_size_null;

	bool expect_test_fail;

	unsigned int num_args_in;
	struct test_arg* args_in;

	unsigned int num_args_out;
	struct test_arg* args_out;
};

unsigned int num_tests = 0;
struct test* tests = NULL;

struct test create_test()
{
	struct test t = {
		.name = NULL,

		.kernel_name = NULL,
		.work_dimensions = work_dimensions,
		//.global_work_size = global_work_size,
		//.local_work_size = local_work_size,
		.local_work_size_null = local_work_size_null,

		.expect_test_fail = expect_test_fail,

		.num_args_in = 0,
		.args_in = NULL,

		.num_args_out = 0,
		.args_out = NULL,
	};

	memcpy(t.global_work_size, global_work_size, sizeof(global_work_size));
	memcpy(t.local_work_size, local_work_size, sizeof(local_work_size));

	return t;
}

void
add_test(struct test t)
{
	add_dynamic_array((void**)&tests, &num_tests, sizeof(struct test), &t);
}

bool
add_test_arg(struct test* t, struct test_arg ta, bool arg_in)
{
	unsigned i;
	struct test_arg* this_args;
	size_t num_this_args;
	struct test_arg* other_args;
	size_t num_other_args;

	if(arg_in) {
		this_args = t->args_in;
		num_this_args = t->num_args_in;
		other_args = t->args_out;
		num_other_args = t->num_args_out;
	} else {
		this_args = t->args_out;
		num_this_args = t->num_args_out;
		other_args = t->args_in;
		num_other_args = t->num_args_in;
	}

	// Check that argument is new
	for(i = 0; i < num_this_args; i++) {
		struct test_arg ta_this = this_args[i];

		if(ta.index == ta_this.index) {
			fprintf(stderr,
			        "Invalid configuration, %s argument with index %d is already defined\n",
			        arg_in ? "In" : "Out",
			        ta.index);
			return false;
		}
	}
	// Check that types correspond
	for(i = 0; i < num_other_args; i++) {
		struct test_arg ta_other = other_args[i];

		if(ta.index == ta_other.index) {
			if(ta.type != ta_other.type) {
				fprintf(stderr,
				        "Invalid configuration, in argument at index %d isn't compatible with out argument\n",
				        ta.index);
				return false;
			}
			if(ta.size != ta_other.size) {
				fprintf(stderr,
				        "Invalid configuration, Size of in argument (%zu) at index %d isn't the same as size of out argument (%zu)\n",
				        arg_in ? ta.size : ta_other.size,
				        ta.index,
				        arg_in ? ta_other.size : ta.size);
				return false;
			}
		}
	}

	if(arg_in) {
		add_dynamic_array((void**)&t->args_in,
		                  &t->num_args_in,
		                  sizeof(struct test_arg),
		                  &ta);
	} else {
		add_dynamic_array((void**)&t->args_out,
		                  &t->num_args_out,
		                  sizeof(struct test_arg),
		                  &ta);
	}

	return true;
}

bool
add_test_arg_in(struct test* t, struct test_arg ta)
{
	return add_test_arg(t, ta, true);
}

bool
add_test_arg_out(struct test* t, struct test_arg ta)
{
	return add_test_arg(t, ta, false);
}

void
free_tests()
{
	unsigned i,j;

	for(i = 0; i < num_tests; i++) {
		for(j = 0; j < tests[i].num_args_in; j++) {
			free(tests[i].args_in[j].value);
		}
		free(tests[i].args_in);
		for(j = 0; j < tests[i].num_args_out; j++) {
			free(tests[i].args_out[j].value);
		}
		free(tests[i].args_out);
	}
}

/* Strings */
unsigned int num_dynamic_strs = 0;
char** dynamic_strs = NULL;

void
add_dynamic_str(char* str)
{
	add_dynamic_array((void**)&dynamic_strs, &num_dynamic_strs, sizeof(char*), &str);
}

char*
add_dynamic_str_copy(const char* src)
{
	char* dst = malloc((strlen(src)+1) * sizeof(char));

	strcpy(dst, src);
	add_dynamic_str(dst);

	return dst;
}

void
free_dynamic_strs()
{
	if(dynamic_strs != NULL) {
		unsigned i;

		for(i = 0; i < num_dynamic_strs; i++) {
			free(dynamic_strs[i]);
		}

		free(dynamic_strs);
	}
}

/* Clean */

void
clean(const int argc,
      const char** argv,
      const struct piglit_cl_program_test_config* config)
{
	free_dynamic_strs();
	free_tests();
}

NORETURN void
exit_report_result(enum piglit_result result)
{
	free_dynamic_strs();
	free_tests();
	piglit_report_result(result);
}

/* Regex functions */

bool
regex_get_matches(const char* src,
                  const char* pattern,
                  regmatch_t* pmatch,
                  size_t size,
                  int cflags)
{
	int errcode;
	regex_t r;

	/* Build regex */
	errcode = regcomp(&r, pattern, REG_EXTENDED | cflags);
	if(errcode) {
		fprintf(stderr, "Invalid regular expression: '%s'\n", pattern);
		return false;
	}

	/* Match regex and if pmatch != NULL && size > 0 return matched */
	if(pmatch == NULL || size == 0) {
		errcode = regexec(&r, src, 0, NULL, 0);
	} else {
		errcode = regexec(&r, src, size, pmatch, 0);
	}

	regfree(&r);

	return errcode == 0;
}

bool
regex_get_match_str(char** dst,
                    const char* src,
                    regmatch_t* pmatch,
                    unsigned int index)
{
	size_t size = pmatch[index].rm_eo - pmatch[index].rm_so;

	if(size > 0) {
		*dst = malloc((size+1) * sizeof(char));
		strncpy(*dst, src+pmatch[index].rm_so, size);
		(*dst)[size] = '\0';

		return true;
	}

	return false;
}

bool
regex_get_str(char** dst,
              const char* src,
              const char* pattern,
              unsigned int index,
              int cflags)
{
	regmatch_t *pmatch = calloc(index+1, sizeof(*pmatch));
	bool ret = false;

	if(regex_get_matches(src, pattern, pmatch, index+1, cflags)) {
		ret = regex_get_match_str(dst, src, pmatch, index);
	}

	free(pmatch);
	return ret;
}

bool
regex_match(const char* src, const char* pattern)
{
	return regex_get_matches(src, pattern, NULL, 0, REG_NEWLINE);
}

bool
regex_section(const char* src, char** section)
{
	regmatch_t pmatch[2];

	if(regex_get_matches(src, REGEX_SECTION, pmatch, 2, REG_NEWLINE)) {
		return regex_get_match_str(section, src, pmatch, 1);
	}

	return false;
}

bool
regex_key_value(const char* src, char** key, char** value)
{
	regmatch_t pmatch[3];

	if(regex_get_matches(src, REGEX_KEY_VALUE, pmatch, 3, REG_NEWLINE)) {
		*key = NULL;
		*value = NULL;
		if(   regex_get_match_str(key, src, pmatch, 1)
		   && regex_get_match_str(value, src, pmatch, 2)) {
			return true;
		} else {
			free(*key);
			free(*value);
		}
	}

	return false;
}

bool
get_bool(const char* src)
{
	if(regex_match(src, REGEX_FULL_MATCH(REGEX_BOOL_TRUE))) {
		return true;
	} else if(regex_match(src, REGEX_FULL_MATCH(REGEX_BOOL_FALSE))) {
		return false;
	} else {
		fprintf(stderr,
		        "Invalid configuration, could not convert to bool: %s\n",
		        src);
		exit_report_result(PIGLIT_WARN);
		return false;
	}
}

int64_t
get_int(const char* src)
{
	if(regex_match(src, REGEX_FULL_MATCH(REGEX_UINT))) {
		return strtoull(src, NULL, 0);
	} else if(regex_match(src, REGEX_FULL_MATCH(REGEX_INT))) {
		return strtoll(src, NULL, 0);
	} else {
		fprintf(stderr,
		        "Invalid configuration, could not convert to long: %s\n",
		        src);
		exit_report_result(PIGLIT_WARN);
		return -1;
	}
}

uint64_t
get_uint(const char* src)
{
	if(regex_match(src, REGEX_FULL_MATCH(REGEX_UINT))) {
		return strtoull(src, NULL, 0);
	} else {
		fprintf(stderr,
		        "Invalid configuration, could not convert to ulong: %s\n",
		        src);
		exit_report_result(PIGLIT_WARN);
		return 0;
	}
}

double
get_float(const char* src)
{
	if(regex_match(src, REGEX_FULL_MATCH(REGEX_FLOAT))) {
		if(regex_match(src, REGEX_FULL_MATCH(REGEX_PNAN))) {
			return NAN;
		} else if(regex_match(src, REGEX_FULL_MATCH(REGEX_NNAN))) {
			return -NAN;
		} else if(regex_match(src, REGEX_FULL_MATCH(REGEX_PINF))) {
			return INFINITY;
		} else if(regex_match(src, REGEX_FULL_MATCH(REGEX_NINF))) {
			return -INFINITY;
		} else {
			return strtod(src, NULL);
		}
	} else {
		fprintf(stderr,
		        "Invalid configuration, could not convert to double: %s\n",
		        src);
		exit_report_result(PIGLIT_WARN);
		return 0;
	}
}

size_t
get_array_length(const char* src)
{
	size_t size = 0;

	if(regex_match(src, REGEX_FULL_MATCH(REGEX_ARRAY))) {
		regmatch_t pmatch[1];

		while(regex_get_matches(src,
		                        REGEX_ARRAY_VALUE REGEX_ARRAY_DELIMITER,
		                        pmatch,
		                        1,
		                        REG_NEWLINE)) {
			src += pmatch[0].rm_eo;
			size++;
		}
		if(regex_match(src, REGEX_ARRAY_VALUE)) {
			size++;
		}
	} else {
		fprintf(stderr,
		        "Invalid configuration, could not convert to an array: %s\n",
		        src);
		exit_report_result(PIGLIT_WARN);
	}

	return size;
}

size_t
get_array(const char* src, void** array, size_t size, char* array_pattern)
{
	bool regex_matched;
	size_t i = 0;
	size_t actual_size;
	char* type;
	char* value;
	char* value_pattern;
	regmatch_t pmatch[2];

	actual_size = get_array_length(src);

	if(!strcmp(array_pattern, REGEX_BOOL_ARRAY)) {
		type = "bool";
		value_pattern = REGEX_BOOL REGEX_ARRAY_DELIMITER;
		*(bool**)array = malloc(actual_size * sizeof(bool));
		regex_matched = regex_match(src, REGEX_FULL_MATCH(REGEX_BOOL_ARRAY));
	} else if(!strcmp(array_pattern, REGEX_INT_ARRAY)) {
		type = "long";
		value_pattern = REGEX_INT REGEX_ARRAY_DELIMITER;
		*(int64_t**)array = malloc(actual_size * sizeof(int64_t));
		regex_matched = regex_match(src, REGEX_FULL_MATCH(REGEX_INT_ARRAY));
	} else if(!strcmp(array_pattern, REGEX_UINT_ARRAY)) {
		type = "ulong";
		value_pattern = REGEX_UINT REGEX_ARRAY_DELIMITER;
		*(uint64_t**)array = malloc(actual_size * sizeof(uint64_t));
		regex_matched = regex_match(src, REGEX_FULL_MATCH(REGEX_UINT_ARRAY));
	} else if(!strcmp(array_pattern, REGEX_FLOAT_ARRAY)) {
		type = "double";
		value_pattern = REGEX_FLOAT REGEX_ARRAY_DELIMITER;
		*(double**)array = malloc(actual_size * sizeof(double));
		regex_matched = regex_match(src, REGEX_FULL_MATCH(REGEX_FLOAT_ARRAY));
	} else {
		fprintf(stderr,
		        "Internal error, invalid array pattern: %s\n",
		        array_pattern);
		exit_report_result(PIGLIT_WARN);
	}

	if(size > 0 && actual_size != size) {
		fprintf(stderr,
		        "Invalid configuration, could not convert %s[%zu] to %s[%zu]: %s\n",
		        type, actual_size, type, size, src);
		exit_report_result(PIGLIT_WARN);
	}

	if(regex_match(src, REGEX_FULL_MATCH(REGEX_NULL))) {
		free(*array);
		*array = NULL;
		return 0;
	}

	if(!regex_matched) {
		fprintf(stderr,
		        "Invalid configuration, could not convert to %s array: %s\n",
		        type, src);
		exit_report_result(PIGLIT_WARN);
	}

	while(regex_get_matches(src, value_pattern, pmatch, 2, 0)) {
		if(regex_get_match_str(&value, src, pmatch, 1)) {
			if(!strcmp(array_pattern, REGEX_BOOL_ARRAY)) {
				(*(bool**)array)[i] = get_bool(value);
			} else if(!strcmp(array_pattern, REGEX_INT_ARRAY)) {
				(*(int64_t**)array)[i] = get_int(value);
			} else if(!strcmp(array_pattern, REGEX_UINT_ARRAY)) {
				(*(uint64_t**)array)[i] = get_uint(value);
			} else if(!strcmp(array_pattern, REGEX_FLOAT_ARRAY)) {
				(*(double**)array)[i] = get_float(value);
			}
			free(value);
		} else {
			fprintf(stderr,
			        "Invalid configuration, could not read %s on index %zu: %s\n",
			        type, i, src);
			exit_report_result(PIGLIT_WARN);
		}

		src += pmatch[0].rm_eo;
		i++;
	}

	if(!strcmp(array_pattern, REGEX_BOOL_ARRAY)) {
		(*(bool**)array)[i] = get_bool(src);
	} else if(!strcmp(array_pattern, REGEX_INT_ARRAY)) {
		(*(int64_t**)array)[i] = get_int(src);
	} else if(!strcmp(array_pattern, REGEX_UINT_ARRAY)) {
		(*(uint64_t**)array)[i] = get_uint(src);
	} else if(!strcmp(array_pattern, REGEX_FLOAT_ARRAY)) {
		(*(double**)array)[i] = get_float(src);
	}

	return actual_size;
}

size_t
get_bool_array(const char* src, bool** array, size_t size)
{
	return get_array(src, (void**)array, size, REGEX_BOOL_ARRAY);
}

size_t
get_int_array(const char* src, int64_t** array, size_t size)
{
	return get_array(src, (void**)array, size, REGEX_INT_ARRAY);
}

size_t
get_uint_array(const char* src, uint64_t** array, size_t size)
{
	return get_array(src, (void**)array, size, REGEX_UINT_ARRAY);
}

size_t
get_float_array(const char* src, double** array, size_t size)
{
	return get_array(src, (void**)array, size, REGEX_FLOAT_ARRAY);
}

/* Help */

void
print_usage(const int argc, const char** argv)
{
	printf("Usage:\n" \
	       "  %s [options] CONFIG.program_test\n"
	       "  %s [options] [-config CONFIG.program_test] PROGRAM.cl|PROGRAM.bin\n"
	       "\n"
	       "Notes:\n"
	       "  - If CONFIG is not specified and PROGRAM has a comment config then a\n"
	       "    comment config is used.\n"
	       "  - If there is no CONFIG or comment config, then the program is only\n"
	       "    tested to build properly.\n",
	       argv[0], argv[0]);
}

void
print_usage_and_warn(const int argc, const char** argv, const char* fmt, ...)
{
	va_list args;

	fprintf(stderr, "ERROR: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	printf("\n\n");
	print_usage(argc, argv);

	exit_report_result(PIGLIT_WARN);
}

/* Parse configuration */

size_t
get_section_content(const char* src, char** content)
{
	size_t size = 0;
	size_t section_name_length = strcspn(src, "\n\0");
	regmatch_t pmatch[2];

	if(src[section_name_length] != '\0') {
		src += section_name_length+1;
	} else {
		src += section_name_length;
	}

	if(regex_get_matches(src, REGEX_SECTION, pmatch, 2, REG_NEWLINE)) { // found next section
		size = pmatch[1].rm_so-1;
	} else { // no next section
		size = strlen(src);
	}

	*content = malloc((size+1) * sizeof(char));
	strncpy(*content, src, size);
	(*content)[size] = '\0';

	return size;
}

void
get_test_arg_value(struct test_arg* test_arg, const char* value, size_t length)
{
	size_t i; // index in array
	size_t c; // component in element
	size_t ra; // offset from the beginning of array
	size_t rb; // offset from the beginning of bufffer

	int64_t* int_array = NULL;
	uint64_t* uint_array = NULL;
	double* float_array = NULL;

	test_arg->value = malloc(test_arg->size);

	/*
	 * We fill the buffer with calculating the right offsets in the buffer (rb)
	 * and in the array (ra). Buffers of type3 have have stride of
	 * 4*sizeof(type) while array has a stride of 3*sizeof(parsed_type).
	 * Also the array index is inside length parameter modulo, so we can fill
	 * the buffer with repeateable values.
	 */
#define CASE(enum_type, cl_type, get_func, array)                   \
	case enum_type:                                                 \
		get_func(value, &array, length);                            \
		for(i = 0; i < test_arg->length; i++) {                     \
			for(c = 0; c < test_arg->cl_size; c++) {                \
				ra = i*test_arg->cl_size + c;                       \
				rb = i*test_arg->cl_mem_size + c;                   \
				((cl_type*)test_arg->value)[rb] = array[ra%length]; \
			}                                                       \
		}                                                           \
		break;

	switch(test_arg->cl_type) {
		CASE(TYPE_CHAR,   cl_char,    get_int_array,    int_array)
		CASE(TYPE_UCHAR,  cl_uchar,   get_uint_array,   uint_array)
		CASE(TYPE_SHORT,  cl_short,   get_int_array,    int_array)
		CASE(TYPE_USHORT, cl_ushort,  get_uint_array,   uint_array)
		CASE(TYPE_INT,    cl_int,     get_int_array,    int_array)
		CASE(TYPE_UINT,   cl_uint,    get_uint_array,   uint_array)
		CASE(TYPE_LONG,   cl_long,    get_int_array,    int_array)
		CASE(TYPE_ULONG,  cl_ulong,   get_uint_array,   uint_array)
		CASE(TYPE_FLOAT,  cl_float,   get_float_array,  float_array)
		CASE(TYPE_DOUBLE,  cl_double,   get_float_array,  float_array)
	}

#undef CASE

	free(int_array);
	free(uint_array);
	free(float_array);
}

void
get_test_arg_tolerance(struct test_arg* test_arg, const char* tolerance_str)
{
	regmatch_t pmatch[2];
	char* value_str = NULL;

        if(regex_get_matches(tolerance_str,
	                     REGEX_ARG_TOLERANCE_ULP,
	                     pmatch,
	                     2,
	                     REG_NEWLINE)) {
		regex_get_match_str(&value_str, tolerance_str, pmatch, 1);
		switch(test_arg->cl_type) {
		case TYPE_FLOAT:
		case TYPE_DOUBLE:
			test_arg->ulp = get_uint(value_str);
			return;
		default:
			fprintf(stderr, "ulp not value for integer types\n");
			exit_report_result(PIGLIT_WARN);
                }
	}

	if(regex_get_matches(tolerance_str,
	                     REGEX_ARG_TOLERANCE,
	                     pmatch,
	                     2,
	                     REG_NEWLINE)) {
		regex_get_match_str(&value_str, tolerance_str, pmatch, 1);

		switch(test_arg->cl_type) {
		case TYPE_CHAR:
		case TYPE_SHORT:
		case TYPE_INT:
		case TYPE_LONG:
			test_arg->toli = get_int(value_str);
			break;
		case TYPE_UCHAR:
		case TYPE_USHORT:
		case TYPE_UINT:
		case TYPE_ULONG:
			test_arg->tolu = get_uint(value_str);
			break;
		case TYPE_FLOAT:
		case TYPE_DOUBLE: {
			float value = get_float(value_str);
			test_arg->ulp = *((uint64_t*)(&value));
			break;
			}
		}

		free(value_str);
	} else {
		fprintf(stderr,
		        "Invalid configuration, could not parse tolerance: %s\n",
		        tolerance_str);
		exit_report_result(PIGLIT_WARN);
	}
}

void
get_test_arg(const char* src, struct test* test, bool arg_in)
{
	regmatch_t pmatch[5];
	char* index_str = NULL;
	char* type = NULL;
	char* value = NULL;
	struct test_arg test_arg = create_test_arg();

	/* Get matches */
	if(regex_get_matches(src, REGEX_FULL_MATCH(REGEX_ARG_VALUE),
	                     pmatch, 5, REG_NEWLINE)) { // value
		// do nothing
	} else if(regex_get_matches(src, REGEX_FULL_MATCH(REGEX_ARG_BUFFER),
	                            pmatch, 5, REG_NEWLINE)) { // buffer
		// do nothing
	} else {
		fprintf(stderr,
		        "Invalid configuration, invalid test argument: %s\n",
		        src);
		exit_report_result(PIGLIT_WARN);
	}

	/* Get index */
	regex_get_match_str(&index_str, src, pmatch, 1);
	test_arg.index = get_int(index_str);
	free(index_str);

	/* Set type, cl_size, cl_mem_size and size (partially for buffers) */
	regex_get_match_str(&type, src, pmatch, 2);
	if(regex_match(type, "[[:digit:]]+")) {
		char* type_size_str;
		regex_get_str(&type_size_str, type, "[[:digit:]]+", 0, REG_NEWLINE);
		test_arg.cl_size = get_int(type_size_str);
		test_arg.cl_mem_size = test_arg.cl_size != 3 ? test_arg.cl_size : 4; // test if we have type3
		free(type_size_str);
	} else {
		test_arg.cl_size = 1;
		test_arg.cl_mem_size = 1;
	}

#define IF(regex_type, enum_type, main_type)                       \
	if(regex_match(type, REGEX_FULL_MATCH(regex_type))) {          \
		test_arg.cl_type = enum_type;                              \
		test_arg.size = sizeof(main_type) * test_arg.cl_mem_size;  \
	}
#define ELSEIF(regex_type, enum_type, main_type) \
	else IF(regex_type, enum_type, main_type)

	IF    (REGEX_TYPE_CHAR,   TYPE_CHAR,   cl_char)
	ELSEIF(REGEX_TYPE_UCHAR,  TYPE_UCHAR,  cl_uchar)
	ELSEIF(REGEX_TYPE_SHORT,  TYPE_SHORT,  cl_short)
	ELSEIF(REGEX_TYPE_USHORT, TYPE_USHORT, cl_ushort)
	ELSEIF(REGEX_TYPE_INT,    TYPE_INT,    cl_int)
	ELSEIF(REGEX_TYPE_UINT,   TYPE_UINT,   cl_uint)
	ELSEIF(REGEX_TYPE_LONG,   TYPE_LONG,   cl_long)
	ELSEIF(REGEX_TYPE_ULONG,  TYPE_ULONG,  cl_ulong)
	ELSEIF(REGEX_TYPE_FLOAT,  TYPE_FLOAT,  cl_float)
	ELSEIF(REGEX_TYPE_DOUBLE,  TYPE_DOUBLE,  cl_double)

#undef IF
#undef ELSEIF

	free(type);

	/* Get arg type, size and value */
	if(regex_match(src, REGEX_FULL_MATCH(REGEX_ARG_VALUE))) { // value
		/* Values are only allowed for in arguments */
		if(!arg_in) {
			fprintf(stderr,
			        "Invalid configuration, out arguments can only be buffers: %s\n",
			        src);
			exit_report_result(PIGLIT_WARN);
		}

		/* Set arg type */
		test_arg.type = TEST_ARG_VALUE;

		/* Set length */
		test_arg.length = 1;

		/* Get value */
		regex_get_match_str(&value, src, pmatch, 3);
		if(regex_match(value, REGEX_FULL_MATCH(REGEX_NULL))) {
			test_arg.value = NULL;
		} else {
			get_test_arg_value(&test_arg, value, test_arg.cl_size);
		}
		free(value);
	} else if(regex_match(src, REGEX_FULL_MATCH(REGEX_ARG_BUFFER))) { // buffer
		char* array_length_str = NULL;
		const char* tolerance_str = NULL;

		/* Set arg type */
		test_arg.type = TEST_ARG_BUFFER;

		/* Set length */
		regex_get_match_str(&array_length_str, src, pmatch, 3);
		test_arg.length = get_int(array_length_str);
		free(array_length_str);

		/* Set size */
		test_arg.size = test_arg.size * test_arg.length;

		/* Set tolerance */
		tolerance_str = src+pmatch[4].rm_eo;
		if(regex_match(tolerance_str, REGEX_ARG_TOLERANCE)) {
			if(arg_in) {
				fprintf(stderr,
				        "Invalid configuration, in argument buffer can't have tolerance: %s\n",
				        src);
				exit_report_result(PIGLIT_WARN);
			}
			get_test_arg_tolerance(&test_arg, tolerance_str);
		}

		/* Get value */
		regex_get_match_str(&value, src, pmatch, 4);
		if(regex_match(value, REGEX_FULL_MATCH(REGEX_NULL))) {
			test_arg.value = NULL;
			if(!arg_in) {
				fprintf(stderr,
				        "Invalid configuration, out argument buffer value can not be NULL: %s\n",
				        src);
				exit_report_result(PIGLIT_WARN);
			}
		} else {
			if(regex_match(value, REGEX_FULL_MATCH(REGEX_RANDOM))) {
				test_arg.value = malloc(test_arg.size);
				if(!arg_in) {
					fprintf(stderr,
					        "Invalid configuration, out argument buffer can not be random: %s\n",
					        src);
					exit_report_result(PIGLIT_WARN);
				}
			} else if(regex_match(value, REGEX_FULL_MATCH(REGEX_REPEAT))) {
				regmatch_t rmatch[3];
				char* repeat_value_str;

				regex_get_matches(value, REGEX_REPEAT, rmatch, 3, 0);
				regex_get_match_str(&repeat_value_str, value, rmatch, 2);

				get_test_arg_value(&test_arg,
				                   repeat_value_str,
				                   get_array_length(repeat_value_str));

				free(repeat_value_str);
			} else if(regex_match(value, REGEX_ARRAY)) {
				get_test_arg_value(&test_arg,
				                   value,
				                   test_arg.length * test_arg.cl_size);
			}
		}
		free(value);
	}

	if(arg_in) {
		if(!add_test_arg_in(test, test_arg)) {
			fprintf(stderr,
			        "Invalid configuration, could not add in argument: %s\n",
			        src);
			exit_report_result(PIGLIT_WARN);
		}
	} else {
		if(!add_test_arg_out(test, test_arg)) {
			fprintf(stderr,
			        "Invalid configuration, could not add out argument: %s\n",
			        src);
			exit_report_result(PIGLIT_WARN);
		}
	}
}

/**
 * Helper function for parsing a test name and checking for illegal characters.
 */
static char*
parse_name(const char *input)
{
	char *name = add_dynamic_str_copy(input);
	int error;
	regex_t regex;
	regmatch_t pmatch[1];


	if ((error = regcomp(&regex, "\\([/%]\\)",0))) {
		char errbuf[100];
		regerror(error, &regex, errbuf, 100);
		fprintf(stderr, "Failed to compile regex for parse_name():%s\n", errbuf);
		return NULL;
	}

	if (!regexec(&regex, input, 1, pmatch, 0)) {
		char bad_char = *(input + pmatch[0].rm_so);
		fprintf(stderr,	"Illegal character in test name '%s': %c\n",
							input, bad_char);
		return NULL;
	}

	regfree(&regex);
	return name;
}

void
parse_config(const char* config_str,
             struct piglit_cl_program_test_config* config)
{
	const char* pch;
	size_t length = strlen(config_str);

	char* line = NULL;
	char* section = NULL;
	char* key = NULL;
	char* value = NULL;

	struct test* test = NULL;

	bool config_found = false;
	bool test_found = false;

	enum state_t {
		SECTION_NONE,
		SECTION_CONFIG,
		SECTION_TEST,
	} state = SECTION_NONE;

	/* parse config string by each line */
	pch = config_str;
	while(pch < (config_str+length)) {
		regmatch_t pmatch[2];
		size_t line_length;

		/* Get line */
		regex_get_matches(pch, REGEX_LINE, pmatch, 2, REG_NEWLINE);
		line_length = pmatch[0].rm_eo - pmatch[0].rm_so;
		if(!regex_get_match_str(&line, pch, pmatch, 1)) {
			/* Line is empty */
			pch += line_length + 1;
			continue;
		}

		/* Get more lines if it is a multiline */
		if(   regex_match(line, REGEX_KEY_VALUE)
		   && regex_match(line, REGEX_MULTILINE)) {
			char* multiline = malloc(sizeof(char));
			multiline[0] = '\0';

			free(line); line = NULL;

			while((pch <= (config_str+length))) {
				regmatch_t mmatch[2];
				char* old_multiline = multiline;
				char* new_multiline;

				/* Get line */
				regex_get_matches(pch, REGEX_LINE, pmatch, 2, REG_NEWLINE);
				line_length = pmatch[0].rm_eo - pmatch[0].rm_so;
				if(!regex_get_match_str(&line, pch, pmatch, 1)) {
					/* Line is empty */
					break;
				}

				if(regex_match(line, REGEX_MULTILINE)) { // not last line
					regex_get_matches(line, REGEX_MULTILINE,
					                  mmatch, 2, REG_NEWLINE);
					if(!regex_get_match_str(&new_multiline, pch, mmatch, 1)) {
						/* Multiline is empty */
						pch += line_length + 1;
						continue;
					}

					multiline = malloc(strlen(old_multiline) + strlen(new_multiline) + 1);
					strncpy(multiline, old_multiline, strlen(old_multiline));
					strcpy(multiline+strlen(old_multiline), new_multiline);

					pch += line_length+1;

					free(line); free(old_multiline); free(new_multiline);
				} else { // last line
					multiline = malloc(strlen(old_multiline) + strlen(line) + 1);
					strncpy(multiline, old_multiline, strlen(old_multiline));
					strcpy(multiline+strlen(old_multiline), line);

					free(line); free(old_multiline);
					break;
				}
			}

			line = multiline;
		}

		/* parse line */
		if(regex_section(line, &section)) { // SECTION
			if(regex_match(section, "^config$")) { // config
				if(config_found) {
					fprintf(stderr, "Invalid configuration, [config] section can be defined only once\n");
					free(section); section = NULL;
					exit_report_result(PIGLIT_WARN);
				}
				if(test_found) {
					fprintf(stderr, "Invalid configuration, [config] section must be declared before any [test] section\n");
					free(section); section = NULL;
					exit_report_result(PIGLIT_WARN);
				}
				config_found = true;
				state = SECTION_CONFIG;
			} else if(regex_match(section, "^test$")) { // test
				if(!config_found) {
					fprintf(stderr, "Invalid configuration, [config] section must be declared before any [test] section\n");
					free(section); section = NULL;
					exit_report_result(PIGLIT_WARN);
				}
				if(config->expect_build_fail) {
					fprintf(stderr, "Invalid configuration, no tests can be defined when expect_build_fail is true\n");
					free(section); section = NULL;
					exit_report_result(PIGLIT_WARN);
				}
				test_found = true;
				add_test(create_test());
				test = &tests[num_tests-1];
				state = SECTION_TEST;
			} else if(regex_match(section, "^program source$")) { // program source
				pch += get_section_content(pch, &config->program_source);
				add_dynamic_str(config->program_source);
				state = SECTION_NONE;
			} else if(regex_match(section, "^program binary$")) { // program binary
				pch += get_section_content(pch, (char**)&config->program_binary);
				add_dynamic_str((char*)config->program_binary);
				state = SECTION_NONE;
			} else {
				fprintf(stderr,
				        "Invalid configuration, configuration has an invalid section: [%s]\n",
				        section);
				free(section); section = NULL;
				exit_report_result(PIGLIT_WARN);
			}

			free(section); section = NULL;
		} else if(regex_key_value(line, &key, &value)) { // KEY : VALUE
			switch(state) {
			case SECTION_NONE:
				fprintf(stderr,
				        "Invalid configuration, this key-value does not belong to any section: %s\n",
				        line);
				free(key); key = NULL;
				free(value); value = NULL;
				exit_report_result(PIGLIT_WARN);
				break;
			case SECTION_CONFIG:
				if(regex_match(key, "^name$")) {
					config->name = parse_name(value);
					if (!config->name) {
						exit_report_result(PIGLIT_FAIL);
					}
				} else if(regex_match(key, "^clc_version_min$")) {
					config->clc_version_min = get_int(value);
				} else if(regex_match(key, "clc_version_max$")) {
					config->clc_version_max = get_int(value);
				} else if(regex_match(key, "^platform_regex$")) {
					config->platform_regex = add_dynamic_str_copy(value);
				} else if(regex_match(key, "^device_regex$")) {
					config->platform_regex = add_dynamic_str_copy(value);
				} else if(regex_match(key, "^require_platform_extensions$")) {
					config->require_platform_extensions =
						add_dynamic_str_copy(value);
				} else if(regex_match(key, "^require_device_extensions$")) {
					config->require_device_extensions =
						add_dynamic_str_copy(value);
				} else if(regex_match(key, "^program_source_file$")) {
					config->program_source_file = add_dynamic_str_copy(value);
				} else if(regex_match(key, "^program_binary_file$")) {
					config->program_binary_file = add_dynamic_str_copy(value);
				} else if(regex_match(key, "^build_options$")) {
					config->build_options = add_dynamic_str_copy(value);
				} else if(regex_match(key, "^kernel_name$")) {
					if(!regex_match(value, REGEX_NULL)) {
						config->kernel_name = add_dynamic_str_copy(value);
					} else {
						config->kernel_name = NULL;
					}
				} else if(regex_match(key, "^expect_build_fail$")) {
					config->expect_build_fail = get_bool(value);
				} else if(regex_match(key, "^expect_test_fail$")) {
					expect_test_fail = get_bool(value);
				} else if(regex_match(key, "^dimensions$")) {
					work_dimensions = get_uint(value);
				} else if(regex_match(key, "^global_size$")) {
					int i;
					uint64_t* int_global_work_size;
					get_uint_array(value, &int_global_work_size, 3);
					for(i = 0; i < 3; i++) {
						global_work_size[i] = int_global_work_size[i];
					}
					free(int_global_work_size);
				} else if(regex_match(key, "^local_size$")) {
					if(!regex_match(value, REGEX_FULL_MATCH(REGEX_NULL))) {
						int i;
						uint64_t* int_local_work_size;
						get_uint_array(value, &int_local_work_size, 3);
						for(i = 0; i < 3; i++) {
							local_work_size[i] = int_local_work_size[i];
						}
						local_work_size_null = false;
						free(int_local_work_size);
					} else {
						local_work_size_null = true;
					}
				} else {
					fprintf(stderr,
					        "Invalid configuration, key '%s' does not belong to a [config] section: %s\n",
					        key, line);
					free(key); key = NULL;
					free(value); value = NULL;
					exit_report_result(PIGLIT_WARN);
				}
				break;
			case SECTION_TEST:
				if(regex_match(key, "^name$")) {
					test->name = parse_name(value);
					if (!test->name) {
						exit_report_result(PIGLIT_FAIL);
					}
				} else if(regex_match(key, "^kernel_name$")) {
					test->kernel_name = add_dynamic_str_copy(value); // test can't have kernel_name == NULL like config section
				} else if(regex_match(key, "^expect_test_fail$")) {
					test->expect_test_fail = get_bool(value);
				} else if(regex_match(key, "^dimensions$")) {
					test->work_dimensions = get_uint(value);
				} else if(regex_match(key, "^global_size$")) {
					int i;
					uint64_t* int_global_work_size;
					get_uint_array(value, &int_global_work_size, 3);
					for(i = 0; i < 3; i++) {
						test->global_work_size[i] = int_global_work_size[i];
					}
					free(int_global_work_size);
				} else if(regex_match(key, "^local_size$")) {
					if(!regex_match(value, REGEX_FULL_MATCH(REGEX_NULL))) {
						int i;
						uint64_t* int_local_work_size;
						get_uint_array(value, &int_local_work_size, 3);
						for(i = 0; i < 3; i++) {
							test->local_work_size[i] = int_local_work_size[i];
						}
						test->local_work_size_null = false;
						free(int_local_work_size);
					} else {
						test->local_work_size_null = true;
					}
				} else if(regex_match(key, "^arg_in$")) {
					get_test_arg(value, test, true);
				} else if(regex_match(key, "^arg_out$")) {
					get_test_arg(value, test, false);
				} else {
					fprintf(stderr,
					        "Invalid configuration, key '%s' does not belong to a [test] section: %s\n",
					        key, line);
					free(key); key = NULL;
					free(value); value = NULL;
					exit_report_result(PIGLIT_WARN);
				}
				break;
			}
			
			free(key); key = NULL;
			free(value); value = NULL;
		} else if(!regex_match(line, REGEX_IGNORE)){ // not WHITESPACE or COMMENT
			fprintf(stderr,
			        "Invalid configuration, configuration could not be parsed: %s\n",
			        line);
			exit_report_result(PIGLIT_WARN);
		}

		/* Go to next line */
		free(line); line = NULL;
		pch += line_length+1;
	}

	if(!config_found) {
		fprintf(stderr, "Invalid configuration, configuration is missing a [config] section.\n");
		exit_report_result(PIGLIT_WARN);
	}
}

/* Get configuration from comment */

char*
get_comment_config_str(const char* src)
{
	char* config_str = NULL;
	regmatch_t pmatch[2];

	if(regex_get_matches(src, REGEX_COMMENT_CONFIG, pmatch, 2, 0)) {
		if(regex_get_match_str(&config_str, src, pmatch, 1)) {
			return config_str;
		}
	}

	return NULL;
}

/* Init */

void
init(const int argc,
     const char** argv,
     struct piglit_cl_program_test_config* config)
{
	const char * main_argument = piglit_cl_get_unnamed_arg(argc, argv, 0);
	const char * config_file = NULL;

	char* config_str = NULL;
	unsigned int config_str_size;
	bool config_arg_present = piglit_cl_is_arg_defined(argc, argv, "config");

	enum main_argument_type_t {
		ARG_CONFIG,
		ARG_SOURCE,
		ARG_BINARY
	} main_argument_type;

	FILE* temp_file;

	int program_count = 0;

	/* Check if arguments are valid */
	// valid main argument
	if(main_argument == NULL) {
		if(argc == 1) {
			print_usage(argc, argv);
			exit_report_result(PIGLIT_WARN);
		} else {
			print_usage_and_warn(argc, argv, "No main argument.");
		}
	}
	if(!regex_match(main_argument, "\\.(cl|program_test|bin)$")) {
		print_usage_and_warn(argc, argv, "Invalid main argument.");
	}
	temp_file = fopen(main_argument, "r");
	if(temp_file == NULL) {
		print_usage_and_warn(argc, argv, "%s does not exist.", main_argument);
	}
	fclose(temp_file);
	// valid config argument
	if(config_arg_present) {
		config_file = piglit_cl_get_arg_value(argc, argv, "config");
		if(!regex_match(config_file, "\\.program_test$")) {
			print_usage_and_warn(argc, argv, "Invalid config argument.");
		}
		temp_file = fopen(config_file, "r");
		if(temp_file == NULL) {
			print_usage_and_warn(argc, argv, "%s does not exist.", config_file);
		}
		fclose(temp_file);
	}
	// no config argument if using .program_test
	if(regex_match(main_argument, "\\.program_test$") && config_arg_present) {
		print_usage_and_warn(argc,
		                     argv,
		                     "Cannot use config argument if main argument is already a config file.");
	}

	/* Get main_argument type and config string */
	if(regex_match(main_argument, "\\.program_test$")) {
		main_argument_type = ARG_CONFIG;

		config_file = main_argument;
		config_str = piglit_load_text_file(config_file, &config_str_size);
	} else if(regex_match(main_argument, "\\.cl$")) {
		main_argument_type = ARG_SOURCE;

		if(config_arg_present) {
			config_file = piglit_cl_get_arg_value(argc, argv, "config");
			config_str = piglit_load_text_file(config_file, &config_str_size);
		} else {
			char* source_str;
			
			config_file = main_argument;
			source_str = piglit_load_text_file(config_file, &config_str_size);
			config_str = get_comment_config_str(source_str);
			
			free(source_str);
		}
	} else if(regex_match(main_argument, "\\.bin$")) {
		main_argument_type = ARG_BINARY;

		config_file = piglit_cl_get_arg_value(argc, argv, "config");
		config_str = piglit_load_text_file(config_file, &config_str_size);
	} else {
		fprintf(stderr, "Ops, this should not happen!\n");
		exit_report_result(PIGLIT_WARN);
	}

	/* Parse test configuration */
	if(config_str != NULL) {
		parse_config(config_str, config);
		free(config_str);
	} else {
		fprintf(stderr, "No configuration found.\n");
	}

	/* Set program */
	switch(main_argument_type) {
	case ARG_CONFIG:
		if(   config->program_source_file != NULL
		   || config->program_binary_file != NULL) {
			const char* program_file = NULL;
			char* main_arg_copy = NULL;
			char* dname = NULL;
			char* full_path_file = NULL;

			if(config->program_source_file != NULL) {
				program_file = config->program_source_file;
			} else {
				program_file = config->program_binary_file;
			}

			main_arg_copy = strdup(main_argument);
			dname = dirname(main_arg_copy);
			full_path_file = malloc(strlen(dname) + strlen(program_file) + 2); // +2 for '/' and '\0'
			strcpy(full_path_file, dname);
			full_path_file[strlen(dname)] = '/';
			strcpy(full_path_file+strlen(dname)+1, program_file);

			if(config->program_source_file != NULL) {
				config->program_source_file = full_path_file;
			} else {
				config->program_binary_file = full_path_file;
			}

			add_dynamic_str(full_path_file);
			free(main_arg_copy);
		}
		break;
	case ARG_SOURCE:
		config->program_source_file = add_dynamic_str_copy(main_argument);
		break;
	case ARG_BINARY:
		config->program_binary_file = add_dynamic_str_copy(main_argument);
		break;
	}

	/* Check if there is one program_* set */
	if(config->program_source != NULL) program_count++;
	if(config->program_source_file != NULL) program_count++;
	if(config->program_binary != NULL) program_count++;
	if(config->program_binary_file != NULL) program_count++;
	if(program_count == 0) {
		fprintf(stderr, "Invalid configuration, no program defined.\n");
		exit_report_result(PIGLIT_WARN);
	} else if (program_count > 1) {
		fprintf(stderr, "Invalid configuration, multiple programs defined.\n");
		exit_report_result(PIGLIT_WARN);
	}

	/* Run test per device */
	config->run_per_device = true;
}

/* Buffer functions */

struct buffer_arg {
	cl_uint index;
	cl_mem buffer;
};

void
free_buffer_args(struct buffer_arg** buffer_args, unsigned int* num_buffer_args)
{
	unsigned i;

	for(i = 0; i < *num_buffer_args; i++) {
		clReleaseMemObject((*buffer_args)[i].buffer);
	}

	free(*buffer_args); *buffer_args = NULL;
	*num_buffer_args = 0;
}

bool
check_test_arg_value(struct test_arg test_arg,
                     void* value)
{
	size_t i; // index in array
	size_t c; // component in element
	size_t ra; // offset from the beginning of parsed array
	size_t rb; // offset from the beginning of buffer

#define CASEI(enum_type, type, cl_type)                                     \
	case enum_type:                                                         \
		for(i = 0; i < test_arg.length; i++) {                              \
			for(c = 0; c < test_arg.cl_size; c++) {                         \
				rb = i*test_arg.cl_mem_size + c;                            \
				if(!piglit_cl_probe_integer(((cl_type*)value)[rb],          \
				                            ((cl_type*)test_arg.value)[rb], \
				                            test_arg.toli)) {               \
					ra = i*test_arg.cl_size + c;                            \
					printf("Error at %s[%zu]\n", type, ra);                 \
					return false;                                           \
				}                                                           \
			}                                                               \
		}                                                                   \
		return true;
#define CASEU(enum_type, type, cl_type)                                      \
	case enum_type:                                                          \
		for(i = 0; i < test_arg.length; i++) {                               \
			for(c = 0; c < test_arg.cl_size; c++) {                          \
				rb = i*test_arg.cl_mem_size + c;                             \
				if(!piglit_cl_probe_uinteger(((cl_type*)value)[rb],          \
				                             ((cl_type*)test_arg.value)[rb], \
				                             test_arg.tolu)) {               \
					ra = i*test_arg.cl_size + c;                             \
					printf("Error at %s[%zu]\n", type, ra);                  \
					return false;                                            \
				}                                                            \
			}                                                                \
		}                                                                    \
		return true;
#define CASEF(enum_type, type, cl_type)                                      \
	case enum_type:                                                          \
		for(i = 0; i < test_arg.length; i++) {                               \
			for(c = 0; c < test_arg.cl_size; c++) {                          \
				rb = i*test_arg.cl_mem_size + c;                             \
				if(!piglit_cl_probe_floating(((cl_type*)value)[rb],          \
				                             ((cl_type*)test_arg.value)[rb], \
				                             test_arg.ulp)) {               \
					ra = i*test_arg.cl_size + c;                             \
					printf("Error at %s[%zu]\n", type, ra);                  \
					return false;                                            \
				}                                                            \
			}                                                                \
		}                                                                    \
		return true;

	switch(test_arg.cl_type) {
		CASEI(TYPE_CHAR,   "char",   cl_char)
		CASEU(TYPE_UCHAR,  "uchar",  cl_uchar)
		CASEI(TYPE_SHORT,  "short",  cl_short)
		CASEU(TYPE_USHORT, "ushort", cl_ushort)
		CASEI(TYPE_INT,    "int",    cl_int)
		CASEU(TYPE_UINT,   "uint",   cl_uint)
		CASEI(TYPE_LONG,   "long",   cl_long)
		CASEU(TYPE_ULONG,  "ulong",  cl_ulong)
		CASEF(TYPE_FLOAT,  "float",  cl_float)
		CASEF(TYPE_DOUBLE,  "double",  cl_double)
	}

#undef CASEF
#undef CASEU
#undef CASEI

	return true;
}

/* Run the kernel test */
enum piglit_result
test_kernel(const struct piglit_cl_program_test_config* config,
            const struct piglit_cl_program_test_env* env,
            struct test test)
{
	enum piglit_result result = PIGLIT_PASS;

	// all
	unsigned j;
	char* kernel_name;
	cl_kernel kernel;

	// setting/validating arguments
	struct buffer_arg* buffer_args = NULL;
	unsigned int  num_buffer_args = 0;

	/* Check if this device supports the local work size. */
	if (!piglit_cl_framework_check_local_work_size(env->device_id,
						test.local_work_size)) {
		return PIGLIT_SKIP;
	}

	/* Create or use apropriate kernel */
	if(test.kernel_name == NULL) {
		kernel_name = config->kernel_name;
		kernel = env->kernel;

		if(config->kernel_name == NULL) {
			printf("No kernel_name defined\n");
			return PIGLIT_WARN;
		} else {
			clRetainKernel(kernel);
		}
	} else {
		kernel_name = test.kernel_name;
		kernel = piglit_cl_create_kernel(env->program, test.kernel_name);

		if(kernel == NULL) {
			printf("Could not create kernel %s\n", kernel_name);
			return PIGLIT_FAIL;
		}
	}

	printf("Using kernel %s\n", kernel_name);

	/* Set kernel args */
	printf("Setting kernel arguments...\n");

	for(j = 0; j < test.num_args_in; j++) {
		bool arg_set = false;
		struct test_arg test_arg = test.args_in[j];

		switch(test_arg.type) {
		case TEST_ARG_VALUE:
			arg_set = piglit_cl_set_kernel_arg(kernel,
			                                   test_arg.index,
			                                   test_arg.size,
			                                   test_arg.value);
			break;
		case TEST_ARG_BUFFER: {
			struct buffer_arg buffer_arg;
			buffer_arg.index = test_arg.index;

			if(test_arg.value != NULL) {
				buffer_arg.buffer = piglit_cl_create_buffer(env->context,
				                                            CL_MEM_READ_WRITE,
				                                            test_arg.size);
				if(   buffer_arg.buffer != NULL
				   && piglit_cl_write_buffer(env->context->command_queues[0],
				                             buffer_arg.buffer,
				                             0,
				                             test_arg.size,
				                             test_arg.value)
				   && piglit_cl_set_kernel_arg(kernel,
				                               buffer_arg.index,
				                               sizeof(cl_mem),
				                               &buffer_arg.buffer)) {
					arg_set = true;
				}
			} else {
				buffer_arg.buffer = NULL;
				arg_set = piglit_cl_set_kernel_arg(kernel,
				                                   buffer_arg.index,
				                                   sizeof(cl_mem),
				                                   NULL);
			}

			if(arg_set) {
				add_dynamic_array((void**)&buffer_args,
				                  &num_buffer_args,
				                  sizeof(struct buffer_arg),
				                  &buffer_arg);
			}
			break;
		}}

		if(!arg_set) {
			printf("Failed to set kernel argument with index %u\n",
			       test_arg.index);
			clReleaseKernel(kernel);
			free_buffer_args(&buffer_args, &num_buffer_args);
			return PIGLIT_FAIL;
		}
	}

	for(j = 0; j < test.num_args_out; j++) {
		bool arg_set = false;
		struct test_arg test_arg = test.args_out[j];

		switch(test_arg.type) {
		case TEST_ARG_VALUE:
			// not accepted by parser
			break;
		case TEST_ARG_BUFFER: {
			unsigned k;
			struct buffer_arg buffer_arg;
			buffer_arg.index = test_arg.index;

			for(k = 0; k < num_buffer_args; k++) {
				if(buffer_args[k].index == buffer_arg.index) {
					arg_set = true;
				}
			}
			if(arg_set) {
				break;
			}

			if(test_arg.value != NULL) {
				buffer_arg.buffer = piglit_cl_create_buffer(env->context,
				                                            CL_MEM_READ_WRITE,
				                                            test_arg.size);
				if(   buffer_arg.buffer != NULL
				   && piglit_cl_set_kernel_arg(kernel,
				                               buffer_arg.index,
				                               sizeof(cl_mem),
				                               &buffer_arg.buffer)) {
					arg_set = true;
				}
			} else {
				buffer_arg.buffer = NULL;
				arg_set = piglit_cl_set_kernel_arg(kernel,
				                                   buffer_arg.index,
				                                   sizeof(cl_mem),
				                                   NULL);
			}

			if(arg_set) {
				add_dynamic_array((void**)&buffer_args,
				                  &num_buffer_args,
				                  sizeof(struct buffer_arg),
				                  &buffer_arg);
			}
			break;
		}}

		if(!arg_set) {
			printf("Failed to set kernel argument with index %u\n",
			       test_arg.index);
			clReleaseKernel(kernel);
			free_buffer_args(&buffer_args, &num_buffer_args);
			return PIGLIT_FAIL;
		}
	}

	/* Execute kernel */
	printf("Running the kernel...\n");

	if(!piglit_cl_execute_ND_range_kernel(env->context->command_queues[0],
	                                      kernel,
	                                      test.work_dimensions,
	                                      test.global_work_size,
	                                      test.local_work_size_null ? NULL : test.local_work_size)) {
		printf("Failed to enqueue the kernel\n");
		clReleaseKernel(kernel);
		free_buffer_args(&buffer_args, &num_buffer_args);
		return PIGLIT_FAIL;
	}

	/* Check results */
	printf("Validating results...\n");

	for(j = 0; j < test.num_args_out; j++) {
		unsigned k;
		bool arg_valid = false;
		struct test_arg test_arg = test.args_out[j];

		switch(test_arg.type) {
		case TEST_ARG_VALUE:
			// Not accepted by parser
			break;
		case TEST_ARG_BUFFER: {
			struct buffer_arg buffer_arg;

			/* Find the right buffer */
			for(k = 0; k < num_buffer_args; k++) {
				if(buffer_args[k].index == test_arg.index) {
					buffer_arg = buffer_args[k];
				}
			}

			if(test_arg.value != NULL) {
				void* read_value = malloc(test_arg.size);

				if(piglit_cl_read_buffer(env->context->command_queues[0],
					                     buffer_arg.buffer,
					                     0,
					                     test_arg.size,
					                     read_value)) {
					arg_valid = true;
					if(check_test_arg_value(test_arg, read_value)) {
						printf(" Argument %u: PASS%s\n",
						                     test_arg.index,
						                     !test.expect_test_fail ? "" : " (not expected)");
						if(test.expect_test_fail) {
							piglit_merge_result(&result, PIGLIT_FAIL);
						}
					} else {
						printf(" Argument %u: FAIL%s\n",
						                     test_arg.index,
						                     !test.expect_test_fail ? "" : " (expected)");
						if(!test.expect_test_fail) {
							piglit_merge_result(&result, PIGLIT_FAIL);
						}
					}
				}

				free(read_value);
			}

			break;
		}}

		if(!arg_valid) {
			printf("Failed to validate kernel argument with index %u\n",
			       test_arg.index);
			clReleaseKernel(kernel);
			free_buffer_args(&buffer_args, &num_buffer_args);
			return PIGLIT_FAIL;
		}
	}

	/* Clean memory used by test */
	clReleaseKernel(kernel);
	free_buffer_args(&buffer_args, &num_buffer_args);
	return result;
}

/* Run test */

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_program_test_config* config,
               const struct piglit_cl_program_test_env* env)
{
	enum piglit_result result = PIGLIT_SKIP;

	unsigned i;

	/* Print building status */
	if(!config->expect_build_fail) {
		printf("Program has been built successfully\n");
	} else {
		printf("Program has failed to build as expected\n");
	}

	/* If num_tests is 0, then we are running a build test and the fact
	 * that we have made it this far means that the program has built
	 * successfully, so we can report PIGLIT_PASS.
	 */
	if (num_tests == 0) {
		result = PIGLIT_PASS;
	}

	/* Run the tests */
	for(i = 0; i< num_tests; i++) {
		enum piglit_result test_result;
		char* test_name = tests[i].name != NULL ? tests[i].name : "";

		printf("> Running kernel test: %s\n", test_name);

		test_result = test_kernel(config, env, tests[i]);
		piglit_merge_result(&result, test_result);

		piglit_report_subtest_result(test_result, "%s", tests[i].name);
	}

	/* Print result */
	if(num_tests > 0) {
		switch(result) {
		case PIGLIT_FAIL:
			printf(">> Some or all of the tests FAILED\n");
			break;
		case PIGLIT_SKIP:
			printf(">> Tests skipped\n");
			break;
		case PIGLIT_WARN:
			printf(">> Some or all of the tests produced a WARNING\n");
			break;
		case PIGLIT_PASS:
			printf(">> All of the tests PASSED\n");
			break;
		}
	}

	return result;
}
