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
 * Test sequences of BufferSubData, ClearBufferSubData, and CopyBufferSubData
 * with partially committed sparse buffers.
 *
 * The same sequence of operations is applied to a sparse buffer and a
 * shadow buffer on the CPU. In the end, the results are read back and compared.
 *
 * Has a stress-test mode: run with -stress N to perform N random operations.
 */

#include "piglit-util-gl.h"

#include <inttypes.h>
#include <time.h>

/* Set this to 1 for verbose logging of operations. */
#define VERBOSE 0

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

PIGLIT_GL_TEST_CONFIG_END

static void
usage(const char *name)
{
	fprintf(stderr, "usage: %s [-stress N] [-seed S]\n", name);
	exit(1);
}

static int sparse_buffer_page_size;

struct buffer_pair {
	GLuint sparse_buffer;
	uint64_t size;
	uint64_t num_pages;

	/* Bitmap indicating which pages are committed. */
	uint8_t *pagemap;

	/* Shadow data buffer, contains the expected data. */
	uint8_t *shadow;

	/* Shadow copy that indicates which bytes contain defined data. */
	uint8_t *defined;
};

static struct buffer_pair *
create_buffers(uint64_t size)
{
	struct buffer_pair *buf = calloc(1, sizeof(*buf));
	if (!buf)
		abort();

	buf->size = size;
	buf->num_pages = (size + sparse_buffer_page_size - 1) / sparse_buffer_page_size;

	buf->shadow = malloc(size);
	buf->defined = calloc(size, 1);

	buf->pagemap = calloc((buf->num_pages + 7) / 8, 1);

	glCreateBuffers(1, &buf->sparse_buffer);

	glNamedBufferStorage(buf->sparse_buffer, size, NULL,
			     GL_DYNAMIC_STORAGE_BIT | GL_SPARSE_STORAGE_BIT_ARB);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return buf;
}

static void
destroy_buffers(struct buffer_pair *buf)
{
	glDeleteBuffers(1, &buf->sparse_buffer);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	free(buf->shadow);
	free(buf->defined);
	free(buf->pagemap);
	free(buf);
}

static void
buffer_page_commitment(struct buffer_pair *buf,
		       uint64_t start_page, uint64_t num_pages, bool commit)
{
	uint64_t commit_start, commit_end;

	assert(start_page < buf->num_pages);
	assert(num_pages <= buf->num_pages - start_page);

	if (VERBOSE)
		printf("%s(%"PRIu64", %"PRIu64", %s)\n", __func__,
		       start_page, num_pages, commit ? "true" : "false");

	commit_start = start_page * sparse_buffer_page_size;
	commit_end = MIN2((start_page + num_pages) * sparse_buffer_page_size,
			  buf->size);

	glNamedBufferPageCommitmentARB(buf->sparse_buffer,
				       commit_start, commit_end - commit_start,
				       commit);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (uint64_t page = start_page; page < start_page + num_pages; ++page) {
		uint8_t *pme = &buf->pagemap[page / 8];
		uint8_t bit = 1 << (page % 8);

		if (!commit) {
			*pme &= ~bit;
			memset(&buf->defined[page * sparse_buffer_page_size],
			       0, sparse_buffer_page_size);
		} else {
			*pme |= bit;
		}
	}
}

static bool
is_byte_committed(struct buffer_pair *buf, uint64_t offset)
{
	uint64_t page = offset / sparse_buffer_page_size;
	return (buf->pagemap[page / 8] & (1 << (page % 8))) != 0;
}

static bool
is_byte_defined(struct buffer_pair *buf, uint64_t offset)
{
	return buf->defined[offset];
}

/* Mark all bytes in the given range as defined (except for those in
 * uncommitted pages).
 */
static void
mark_range_defined(struct buffer_pair *buf, uint64_t offset, uint64_t size)
{
	while (size) {
		uint64_t bytes =
			MIN2(size,
			     sparse_buffer_page_size - (offset % sparse_buffer_page_size));

		if (is_byte_committed(buf, offset))
			memset(&buf->defined[offset], 1, bytes);

		offset += bytes;
		size -= bytes;
	}
}

/* Copy the definedness of the source range to the destination range, except
 * that bytes in uncommitted pages are left undefined.
 */
static void
copy_range_defined(struct buffer_pair *buf, uint64_t src_offset,
		   uint64_t dst_offset, uint64_t size)
{
	while (size) {
		uint64_t bytes =
			MIN3(size,
			     sparse_buffer_page_size - (src_offset % sparse_buffer_page_size),
			     sparse_buffer_page_size - (dst_offset % sparse_buffer_page_size));

		if (is_byte_committed(buf, dst_offset)) {
			if (is_byte_committed(buf, src_offset)) {
				memcpy(&buf->defined[dst_offset],
				       &buf->defined[src_offset],
				       bytes);
			} else {
				memset(&buf->defined[dst_offset], 0, bytes);
			}
		}

		src_offset += bytes;
		dst_offset += bytes;
		size -= bytes;
	}
}

static void
buffers_sub_data(struct buffer_pair *buf, uint64_t offset, uint64_t size)
{
	if (VERBOSE)
		printf("%s(%"PRIu64", %"PRIu64")\n", __func__, offset, size);

	for (uint64_t i = 0; i < size; ++i)
		buf->shadow[offset + i] = rand();

	glNamedBufferSubData(buf->sparse_buffer, offset, size, &buf->shadow[offset]);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	mark_range_defined(buf, offset, size);
}

static void
clear_buffers_sub_data(struct buffer_pair *buf, uint64_t offset, uint64_t size,
		       unsigned elt_size)
{
	uint32_t pattern = rand();
	GLenum internal_format;
	GLenum type;

	if (VERBOSE)
		printf("%s(%"PRIu64", %"PRIu64", %u)\n", __func__, offset,
		       size, elt_size);

	switch (elt_size) {
	case 1:
		internal_format = GL_R8UI;
		type = GL_UNSIGNED_BYTE;

		memset(&buf->shadow[offset], pattern, size);
		break;
	case 2:
		internal_format = GL_R16UI;
		type = GL_UNSIGNED_SHORT;

		for (uint64_t i = 0; i < size; i += 2)
			*(uint16_t*)&buf->shadow[offset + i] = pattern;
		break;
	case 4:
		internal_format = GL_R32UI;
		type = GL_UNSIGNED_INT;

		for (uint64_t i = 0; i < size; i += 4)
			*(uint32_t*)&buf->shadow[offset + i] = pattern;
		break;
	default:
		abort();
	}

	glClearNamedBufferSubData(buf->sparse_buffer, internal_format,
				  offset, size,
				  GL_RED_INTEGER, type, &pattern);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	mark_range_defined(buf, offset, size);
}

static void
copy_buffers_sub_data(struct buffer_pair *buf, uint64_t src_offset,
		      uint64_t dst_offset, uint64_t size)
{
	assert(size <= buf->size);
	assert(src_offset <= buf->size - size);
	assert(dst_offset <= buf->size - size);

	if (VERBOSE)
		printf("%s(%"PRIu64", %"PRIu64", %"PRIu64")\n", __func__,
		       src_offset, dst_offset, size);

	memcpy(&buf->shadow[dst_offset], &buf->shadow[src_offset], size);

	glCopyNamedBufferSubData(buf->sparse_buffer, buf->sparse_buffer,
				 src_offset, dst_offset, size);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	copy_range_defined(buf, src_offset, dst_offset, size);
}

static void
dump_region(struct buffer_pair *buf, uint64_t offset, uint64_t size)
{
	uint64_t end = offset + size;
	uint8_t *readback = malloc(size);

	printf("Dump      Shadow                     Sparse\n");
	/*     "xxxxxxxx: xx xx xx xx xx xx xx xx    xx xx xx xx xx xx xx xx" */

	glGetNamedBufferSubData(buf->sparse_buffer, offset, size, readback);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (uint64_t row = offset & ~7; row < end; row += 8) {
		printf("%08"PRIx64":", row);

		for (unsigned i = 0; i < 8; ++i) {
			if (row + i < offset || row + i >= end) {
				printf("   ");
				continue;
			}

			bool defined = is_byte_defined(buf, row + i);
			bool committed = is_byte_committed(buf, row + i);

			if (defined && !committed)
				printf(" !!"); /* internal error */
			else if (!committed)
				printf(" ..");
			else if (!defined)
				printf(" xx");
			else
				printf(" %02x", buf->shadow[row + i]);
		}

		printf("   ");

		for (unsigned i = 0; i < 8; ++i) {
			if (row + i < offset || row + i >= end)
				printf("   ");
			else
				printf(" %02x", readback[row + i - offset]);
		}

		printf("\n");
	}

	free(readback);
}

static bool
verify_buffers_range(struct buffer_pair *buf, uint64_t offset, uint64_t size)
{
	uint8_t *sparse_readback = malloc(size);
	bool pass = true;

	glGetNamedBufferSubData(buf->sparse_buffer, offset, size, sparse_readback);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (uint64_t i = 0; i < size; ++i) {
		if (is_byte_defined(buf, offset + i)) {
			if (!is_byte_committed(buf, offset + i)) {
				printf("Sanity check failed @ 0x%"PRIx64": defined && !committed\n",
				       offset + i);

				dump_region(buf, (MAX2(offset + i, 8) - 8) & ~7, 64);

				pass = false;
				break;
			}

			if (sparse_readback[i] != buf->shadow[offset + i]) {
				printf("Mismatch in defined region @ 0x%"PRIx64"\n"
				       "  Non-sparse: %08x\n"
				       "  Sparse:     %08x\n",
				       offset + i, buf->shadow[offset + i],
				       sparse_readback[i]);

				dump_region(buf, (MAX2(offset + i, 8) - 8) & ~7, 64);

				pass = false;
				break;
			}
		}
	}

	free(sparse_readback);
	return pass;
}

static bool
verify_buffers(struct buffer_pair *buf)
{
	return verify_buffers_range(buf, 0, buf->size);
}

static bool
run_simple()
{
	struct buffer_pair *buf;
	bool pass = true;

	buf = create_buffers(sparse_buffer_page_size * 5 / 2);

	buffer_page_commitment(buf, 0, 1, true);
	buffer_page_commitment(buf, 2, 1, true);

	/* Test each command in a way that touches the uncommitted region. */
	buffers_sub_data(buf, 0, sparse_buffer_page_size * 5 / 2);

	if (!verify_buffers(buf)) {
		pass = false;
		goto out;
	}
	copy_buffers_sub_data(buf,
			      sparse_buffer_page_size - sparse_buffer_page_size / 8,
			      2 * sparse_buffer_page_size,
			      sparse_buffer_page_size / 4);

	if (!verify_buffers_range(buf, 2 * sparse_buffer_page_size, sparse_buffer_page_size / 4)) {
		pass = false;
		goto out;
	}

	clear_buffers_sub_data(buf, sparse_buffer_page_size - 128, 256, 1);

	if (!verify_buffers_range(buf, sparse_buffer_page_size - 128, 128)) {
		pass = false;
		goto out;
	}

	clear_buffers_sub_data(buf, sparse_buffer_page_size - 64, 128, 2);

	if (!verify_buffers_range(buf, sparse_buffer_page_size - 64, 64)) {
		pass = false;
		goto out;
	}

	clear_buffers_sub_data(buf, sparse_buffer_page_size - 32, 64, 4);

	if (!verify_buffers_range(buf, sparse_buffer_page_size - 32, 32)) {
		pass = false;
		goto out;
	}

	/* Test some small "unaligned" cases. */
	clear_buffers_sub_data(buf, 3, 256, 1);
	clear_buffers_sub_data(buf, 262, 258, 2);
	clear_buffers_sub_data(buf, 644, 260, 4);
	buffers_sub_data(buf, 1025, 257);
	copy_buffers_sub_data(buf, 1, 2051, 257);

	if (!verify_buffers(buf)) {
		pass = false;
		goto out;
	}

out:
	destroy_buffers(buf);
	return pass;
}

static unsigned rand_size(uint64_t max)
{
	max = MIN2(max, 1 << 30);

	unsigned log_max = 0;
	for (unsigned i = MAX2(max, 1) - 1; i; i >>= 1)
		log_max++;

	unsigned log_rand = rand() % (log_max + 1);
	return rand() % (MIN2(max, 1 << log_rand) + 1);
}

static bool
run_stress(unsigned num_iterations)
{
	struct buffer_pair *buf;
	unsigned log_num_pages = 10;
	bool pass = true;

	buf = create_buffers((1 << log_num_pages) * sparse_buffer_page_size);

	/* Initialize a fairly dense random commitment. */
	for (unsigned page = 0; page < (1 << log_num_pages);) {
		unsigned num = MIN2(1 + rand() % 16, (1 << log_num_pages) - page);

		buffer_page_commitment(buf, page, num, true);
		page += num;
		page += 1; /* hole */
	}

	/* Random buffer operations. */
	for (unsigned i = 0; i < num_iterations; ++i) {
		unsigned op = rand() % 8;
		unsigned dst_offset, size;

		printf("Iteration %u\n", i);

		if (op < 2) {
			size = rand_size(buf->size / 2);
			dst_offset = rand() % (buf->size - 2 * size + 1);

			unsigned src_offset = dst_offset + size + rand() % (buf->size - (dst_offset + 2 * size) + 1);

			if (rand() & 1) {
				unsigned tmp = dst_offset;
				dst_offset = src_offset;
				src_offset = tmp;
			}

			copy_buffers_sub_data(buf, src_offset, dst_offset, size);
		} else if (op < 5) {
			static const unsigned elt_sizes[] = { 1, 2, 4 };
			unsigned elt_size = elt_sizes[rand() % 3];
			unsigned nelts = rand_size(buf->size / elt_size);

			dst_offset = elt_size * (rand() % (buf->size / elt_size - nelts + 1));
			size = nelts * elt_size;

			clear_buffers_sub_data(buf, dst_offset, size, elt_size);
		} else {
			size = rand_size(buf->size);
			dst_offset = rand() % (buf->size - size + 1);

			buffers_sub_data(buf, dst_offset, size);
		}

		if (!verify_buffers_range(buf, dst_offset, size)) {
			pass = false;
			break;
		}
	}

	if (!verify_buffers(buf))
		pass = false;

	destroy_buffers(buf);
	return pass;
}

enum piglit_result
piglit_display(void)
{
	/* not reached */
	return PIGLIT_FAIL;
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
