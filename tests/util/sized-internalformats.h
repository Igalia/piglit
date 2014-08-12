/* Copyright © 2011 Intel Corporation
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
 */

/* F=float, UN=unsigned normalized, SN=signed normalized, I=int,
 * U=uint.
 *
 * This list has to match up with the table in sized-internalformats.c.
 */
enum bits_types {
	NONE,

	UN32,
	F32,
	I32,
	U32,

	UN24,

	UN16,
	SN16,
	F16,
	I16,
	U16,

	U10,
	U2,

	UN12,
	UN10,

	UN8,
	SN8,
	I8,
	U8,

	UN6,
	UN5,
	UN4,
	UN3,
	UN2,
	UN1,

	F11,
	F10,
	F9,


	/* Compressed internalformats get treated specially. */
	UCMP,
	SCMP,

	BITS_MAX,
};

enum channel {
	R,
	G,
	B,
	A,
	L,
	I,
	D,
	S,
	CHANNELS,
};

struct sized_internalformat {
	const char *name;
	GLenum token;
	enum bits_types bits[CHANNELS];
};

struct required_format {
	GLenum token;
	int version;
	bool rb_required;
};

extern const struct sized_internalformat sized_internalformats[];
extern const struct required_format required_formats[];

const struct sized_internalformat *
get_sized_internalformat(GLenum token);

int
get_channel_size(const struct sized_internalformat *f, enum channel c);

GLenum
get_channel_type(const struct sized_internalformat *f, enum channel c);

void
print_bits(int size, GLenum type);

bool
valid_for_gl_version(const struct required_format *format, int target_version);

void
setup_required_size_test(int argc, char **argv,
			 struct piglit_gl_test_config *config);
