/*
 * Copyright (c) 2017 Advanced Micro Devices, Inc.
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

/** \file
 *
 * Test sequences of commitment and uncommitment.
 *
 * Whenever a page is newly committed, clear it with a random value. Verify
 * that this value is retained by downloading the buffer contents with
 * GetBufferSubData.
 *
 * Has a stress-test mode: run with -stress N to perform N random commit/uncommit
 * operations.
 */

#include "piglit-util-gl.h"

#include <inttypes.h>
#include <time.h>

/* Set to 1 for verbose logging of operations. */
#define VERBOSE 0

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

PIGLIT_GL_TEST_CONFIG_END

static int sparse_buffer_page_size;

struct sparse_buffer {
	GLuint buffer;
	uint64_t num_pages;
	uint64_t num_committed_pages;

	/* Bitmap indicating which pages are committed. */
	uint8_t *pagemap;

	/* For each page, the value that it was cleared with. */
	uint32_t *pagedata;
};

static struct sparse_buffer *
create_buffer(uint64_t num_pages)
{
	uint64_t size;
	struct sparse_buffer *buf = calloc(1, sizeof(*buf));
	if (!buf)
		abort();

	buf->num_pages = num_pages;
	size = buf->num_pages * sparse_buffer_page_size;

	buf->pagemap = calloc((buf->num_pages + 7) / 8, 1);
	buf->pagedata = malloc(sizeof(*buf->pagedata) * buf->num_pages);

	glCreateBuffers(1, &buf->buffer);

	glNamedBufferStorage(buf->buffer, size, NULL,
			     GL_DYNAMIC_STORAGE_BIT | GL_SPARSE_STORAGE_BIT_ARB);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return buf;
}

static void
destroy_buffer(struct sparse_buffer *buf)
{
	glDeleteBuffers(1, &buf->buffer);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	free(buf->pagedata);
	free(buf->pagemap);
	free(buf);
}

static void
buffer_page_commitment(struct sparse_buffer *buf,
		       uint64_t start_page, uint64_t num_pages, bool commit)
{
	assert(start_page < buf->num_pages);
	assert(num_pages <= buf->num_pages - start_page);

	if (VERBOSE)
		fprintf(stderr, "%s(%"PRIu64", %"PRIu64", %s)\n", __func__,
			start_page, num_pages, commit ? "true" : "false");

	glNamedBufferPageCommitmentARB(buf->buffer,
				       start_page * sparse_buffer_page_size,
				       num_pages * sparse_buffer_page_size,
				       commit);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (uint64_t page = start_page; page < start_page + num_pages; ++page) {
		uint8_t *pme = &buf->pagemap[page / 8];
		uint8_t bit = 1 << (page % 8);

		if (!commit) {
			if (*pme & bit) {
				*pme &= ~bit;
				buf->num_committed_pages--;
			}
		} else {
			if (!(*pme & bit)) {
				*pme |= bit;
				buf->num_committed_pages++;

				buf->pagedata[page] = rand();

				glClearNamedBufferSubData(
					buf->buffer, GL_R32UI,
					page * sparse_buffer_page_size,
					sparse_buffer_page_size,
					GL_RED_INTEGER, GL_UNSIGNED_INT,
					&buf->pagedata[page]);
			}
		}
	}
}

static bool
verify_commitments_range(struct sparse_buffer *buf, uint64_t start_page,
			 uint64_t num_pages)
{
	uint64_t buffer_size = num_pages * sparse_buffer_page_size;
	uint32_t *readback = malloc(buffer_size);
	bool pass = true;

	assert(start_page < buf->num_pages);
	assert(num_pages <= buf->num_pages - start_page);

	if (VERBOSE)
		fprintf(stderr, "%s(%"PRIu64", %"PRIu64")\n", __func__,
			start_page, num_pages);

	glGetNamedBufferSubData(buf->buffer, start_page * sparse_buffer_page_size,
				buffer_size, readback);

	for (uint64_t page = start_page; page < start_page + num_pages; ++page) {
		if (!(buf->pagemap[page / 8] & (1 << (page % 8))))
			continue;

		const uint32_t *p =
			readback + (page - start_page) * sparse_buffer_page_size / sizeof(uint32_t);
		for (unsigned i = 0; i < sparse_buffer_page_size / sizeof(uint32_t); ++i) {
			if (*p != buf->pagedata[page]) {
				printf("Readback value incorrect at page %"PRId64", offset 0x%x\n"
				       "  Expected: %08x\n"
				       "  Actual:   %08x\n",
				       page, 4 * i, buf->pagemap[page], *p);
				pass = false;
				goto out;
			}
		}
	}

out:
	free(readback);
	return pass;
}

static bool
verify_commitments(struct sparse_buffer *buf)
{
	return verify_commitments_range(buf, 0, buf->num_pages);
}

static bool
run_simple()
{
	struct sparse_buffer *buf;
	bool pass = true;

	buf = create_buffer(10);

	/* Just a pretty arbitrary sequence of (un)committings. */
	buffer_page_commitment(buf, 0, 2, true);
	buffer_page_commitment(buf, 8, 2, true);
	if (!verify_commitments(buf)) {
		pass = false;
		goto out;
	}

	buffer_page_commitment(buf, 1, 8, false);

	buffer_page_commitment(buf, 2, 2, true);
	buffer_page_commitment(buf, 4, 2, true);
	buffer_page_commitment(buf, 7, 1, true);
	if (!verify_commitments(buf)) {
		pass = false;
		goto out;
	}

	buffer_page_commitment(buf, 3, 2, false);

	buffer_page_commitment(buf, 0, 10, true);
	if (!verify_commitments(buf)) {
		pass = false;
		goto out;
	}

	buffer_page_commitment(buf, 1, 3, false);
	buffer_page_commitment(buf, 5, 1, false);
	buffer_page_commitment(buf, 8, 2, false);

	if (!verify_commitments(buf)) {
		pass = false;
		goto out;
	}

out:
	destroy_buffer(buf);
	return pass;
}

static bool
run_stress(unsigned num_iterations)
{
	struct sparse_buffer *buf;
	unsigned log_num_pages = 10;
	bool pass = true;

	buf = create_buffer(1 << log_num_pages);

	for (unsigned i = 0; i < num_iterations; ++i) {
		unsigned log_range_pages = 2 + rand() % (log_num_pages - 1);
		unsigned num_range_pages = 1 + rand() % (1 << log_range_pages);
		unsigned start_range = rand() % (buf->num_pages - num_range_pages + 1);
		bool commit = rand() & 1;

		if (!commit) {
			if (!verify_commitments_range(buf, start_range, num_range_pages)) {
				pass = false;
				break;
			}
		}

		buffer_page_commitment(buf, start_range, num_range_pages, commit);
	}

	if (!verify_commitments(buf))
		pass = false;

	destroy_buffer(buf);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	/* not reached */
	return PIGLIT_FAIL;
}

static void
usage(const char *name)
{
	fprintf(stderr, "usage: %s [-stress N] [-seed S]\n", name);
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	int stress_iterations = 0;
	int seed = time(NULL);
	bool pass = true;

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-stress")) {
			++i;
			if (i >= argc)
				usage(argv[0]);

			stress_iterations = atoi(argv[i]);
		} else if (!strcmp(argv[i], "-seed")) {
			++i;
			if (i >= argc)
				usage(argv[0]);

			seed = i;
		} else {
			usage(argv[0]);
		}
	}

	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_sparse_buffer");

	glGetIntegerv(GL_SPARSE_BUFFER_PAGE_SIZE_ARB, &sparse_buffer_page_size);

	if (stress_iterations > 0) {
		printf("Running with seed %d.\n"
		       "To reproduce, run: %s -seed %d\n",
		       seed, argv[0], seed);
		srand(seed);

		pass = run_stress(stress_iterations) && pass;
	} else {
		pass = run_simple() && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
