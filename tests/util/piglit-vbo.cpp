/*
 * Copyright © 2011, 2016 Intel Corporation
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
 * \file piglit-vbo.cpp
 *
 * This file adds the facility for specifying vertex data to piglit
 * tests using a columnar text format, for example:
 *
 *   \verbatim
 *   vertex/vec3/3	foo/uint/1	bar[0]/int/1	bar[1]/int/1
 *   0.0 0.0 0.0	10		0		0	# comment
 *   0.0 1.0 0.0	 5		1		1
 *   1.0 1.0 0.0	 0		0		1
 *   \endverbatim
 *
 * The format consists of a row of column headers followed by any
 * number of rows of data.  Each column header has the form
 * "ATTRNAME[ARRAY_INDEX]/TYPE/COUNT/MATRIX_COLUMN", where ATTRNAME is
 * the name of the vertex attribute to be bound to this column,
 * ARRAY_INDEX is the index, TYPE is the GLSL type of data that
 * follows ("float", "double", "int", "uint" or any derived type like
 * vec2, dmat4, etc.), COUNT is the vector length of the data
 * (e.g. "3" for vec3 data or "2" for mat4x2) and MATRIX_COLUMN is the
 * column number of the data in case of being a matrix. [ARRAY_INDEX]
 * is optional and needs to be specified only in case of array
 * attributes. COUNT is a redundant value that is already specified in
 * the data type but that has been kept for backward
 * compatibility. MATRIX_COLUMN doesn't need to be specified if it is
 * not a matrix column as in the example before. So another example,
 * if you want to specify the data of a mat2x3:
 *
 *   \verbatim
 *   foomatrix/mat2x3/3/0	foomatrix/mat2x3/3/1
 *   0.0 1.0 2.0		3.0 4.0 5.0
 *   6.0 7.0 8.0		9.0 10.0 11.0
 *   \endverbatim
 *
 * The same example, using the deprecated but compatible syntax would
 * be:
 *
 *   \verbatim
 *   foomatrix/float/3/0	foomatrix/float/3/1
 *   0.0 1.0 2.0		3.0 4.0 5.0
 *   6.0 7.0 8.0		9.0 10.0 11.0
 *   \endverbatim
 *
 * The data follows the column headers in space-separated form.  "#"
 * can be used for comments, as in shell scripts.
 *
 * To process textual vertex data, call the function
 * setup_vbo_from_text(), passing the int identifying the linked
 * program, and the string containing the vertex data.  The return
 * value is the number of rows of vertex data found.
 *
 * If an error occurs, setup_vbo_from_text() will print out a
 * description of the error and exit with PIGLIT_FAIL.
 *
 * For the first example above, the call to setup_vbo_from_text() is
 * roughly equivalent to the following GL operations:
 *
 * \code
 * struct vertex_attributes {
 *         GLfloat vertex[3];
 *         GLuint foo;
 *         GLint bar[2];
 * } vertex_data[] = {
 *         { { 0.0, 0.0, 0.0 }, 10, { 0, 0 } },
 *         { { 0.0, 1.0, 0.0 },  5, { 1, 1 } },
 *         { { 1.0, 1.0, 0.0 },  0, { 0, 1 } }
 * };
 * size_t stride = sizeof(vertex_data[0]);
 * GLint vertex_index = glGetAttribLocation(prog, "vertex");
 * GLint foo_index = glGetAttribLocation(prog, "foo");
 * GLint bar_index = glGetAttribLocation(prog, "bar");
 * GLuint buffer_handle;
 * glGenBuffers(1, &buffer_handle);
 * glBindBuffer(GL_ARRAY_BUFFER, buffer_handle);
 * glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), &vertex_data,
 *              GL_STATIC_DRAW);
 * glVertexAttribPointer(vertex_index, 3, GL_FLOAT, GL_FALSE, stride,
 *                       (void *) offsetof(vertex_attributes, vertex));
 * glVertexAttribIPointer(foo_index, 3, GL_UNSIGNED_INT, stride,
 *                        (void *) offsetof(vertex_attributes, foo));
 * glVertexAttribIPointer(bar_index, 3, GL_INT, stride,
 *                        (void *) offsetof(vertex_attributes, bar));
 * glEnableVertexAttribArray(vertex_index);
 * glEnableVertexAttribArray(foo_index);
 * glEnableVertexAttribArray(bar_index);
 * \endcode
 *
 * Which could correspond to a vertex shader of this type:
 *
 * \code
 * in vec3 vertex;
 * in uint foo;
 * in int bar[2];
 *
 * void main()
 * {
 * 	if (bar[0] + bar[1] == foo) {
 * 		gl_Position = vec4(vertex, 1.0);
 * 	} else {
 * 		gl_Position = vec4(1.0, vertex);
 * 	}
 * }
 * \endcode
 */

#include <string>
#include <vector>
#include <errno.h>
#include <ctype.h>

#include "piglit-util.h"
#include "piglit-util-gl.h"
#include "piglit-vbo.h"

/**
 * Currently all the attribute types we support (int, uint, and float)
 * are 4 bytes in width.
 */
const int ATTRIBUTE_SIZE = 4;


/**
 * Convert a type name string to a GLenum.
 */
GLenum
decode_type(const char *type, size_t *rows, size_t *cols)
{
	static struct type_table_entry {
		const char *type; /* NULL means end of table */
		GLenum enum_value;
	} const type_table[] = {
		{ "int",     GL_INT            },
		{ "uint",    GL_UNSIGNED_INT   },
		{ "float",   GL_FLOAT          },
		{ "double",  GL_DOUBLE         },
		{ "ivec",    GL_INT            },
		{ "uvec",    GL_UNSIGNED_INT   },
		{ "vec",     GL_FLOAT          },
		{ "dvec",    GL_DOUBLE         },
		{ "mat",     GL_FLOAT          },
		{ "dmat",    GL_DOUBLE         },
		{ NULL,      0                 }
	};


	for (int i = 0; type_table[i].type; ++i) {
		if (0 == strncmp(type, type_table[i].type, strlen(type_table[i].type))) {
			/* In case of vectors or matrices, let's
			 * calculate rows and columns, if needed.
			 */
			if (i > 3 && (rows || cols)) {
				size_t type_len = strlen(type);
				size_t the_rows = type[type_len - 1] - '0';
				if (rows)
					*rows = the_rows;
				if (cols) {
					/* In case of matrices, let's
					 * calculate the columns.
					 */
					if (i > 7)
						*cols = type[type_len - 2] == 'x' ?
							type[type_len - 3] - '0' : the_rows;
					else
						*cols = 1;
				}
			} else {
				if (rows)
					*rows = 1;
				if (cols)
					*cols = 1;
			}
			return type_table[i].enum_value;
		}
	}

	printf("Unrecognized type: %s\n", type);
	piglit_report_result(PIGLIT_FAIL);
	return 0;
}


/**
 * Description of a vertex attribute, built from its column header
 */
class vertex_attrib_description
{
public:
	vertex_attrib_description(GLuint prog, const char *text);
	bool parse_datum(const char **text, void *data) const;
	void setup(size_t *offset, size_t stride) const;

	/**
	 * Data type of this attribute.
	 */
	GLenum data_type;

	/**
	 * Number of columns in the data type of this attribute.
	 */
	size_t data_type_cols;

	/**
	 * Vector length of this attribute.
	 */
	size_t count;

	/**
	 * Index of the matrix column for this attribute.
	 */
	size_t matrix_index;

	/**
	 * Index of the array for this attribute.
	 */
	size_t array_index;

	/**
	 * Index of this vertex attribute in the linked program.
	 */
	GLuint index;
};


/**
 * Build a vertex_attrib_description from a column header, by looking
 * up the vertex attribute in the linked program and interpreting the
 * type and count parts of the header.
 *
 * If there is a parse failure, print a description of the problem and
 * then exit with PIGLIT_FAIL.
 */
vertex_attrib_description::vertex_attrib_description(GLuint prog,
						     const char *text)
{
	/* Split the column header into
	 * name[array_index]/type/count/matrix_column fields.
	 */
	const char *first_slash = strchr(text, '/');
	if (first_slash == NULL) {
		printf("Column headers must be in the form"
		       " name[array_index]/type/count/matrix_column.\n"
		       "Note: [array_index] and matrix_column are optional.\n"
		       "Got: %s\n",
		       text);
		piglit_report_result(PIGLIT_FAIL);
	}

	std::string name(text, first_slash);

	/* If the attrib is an array, strip the index */
	if (name[name.size() - 1] == ']') {
		std::string::size_type n = name.find('[');
		if (n == std::string::npos) {
			printf("Column header looked like an array"
			       " but couldn't parse it.\n"
			       "Got: %s\n",
			       text);
			piglit_report_result(PIGLIT_FAIL);
		} else {
			this->array_index = strtoul(name.substr(n + 1).c_str(), NULL, 0);
			name.resize(n);
		}
	} else {
		this->array_index = 0;
	}

	const char *second_slash = strchr(first_slash + 1, '/');
	if (second_slash == NULL) {
		printf("Column headers must be in the form"
		       " name[array_index]/type/count/matrix_column.\n"
		       "Note: [array_index] and matrix_column are optional.\n"
		       "Got: %s\n",
		       text);
		piglit_report_result(PIGLIT_FAIL);
	}
	std::string type_str(first_slash + 1, second_slash);
	this->data_type = decode_type(type_str.c_str(),
				      &this->count, &this->data_type_cols);
	char *endptr;
	this->count = strtoul(second_slash + 1, &endptr, 10);

	if (*endptr != '\0') {
		const char *third_slash = strchr(second_slash + 1, '/');
		this->matrix_index = strtoul(third_slash + 1, &endptr, 10);

		if (this->matrix_index < 0 || this->matrix_index > 3) {
			printf("Matrix column index must be between 0 and 3.  Got: %lu\n",
				(unsigned long) this->matrix_index);
			piglit_report_result(PIGLIT_FAIL);
		}

		if (*endptr != '\0') {
			printf("Column headers must be in the form"
			       " name[array_index]/type/count/matrix_column.\n"
			       "Note: [array_index] and matrix_column are optional.\n"
			       "Got: %s\n",
			       text);
			piglit_report_result(PIGLIT_FAIL);
		}
	} else {
		this->matrix_index = 0;
	}

	GLint attrib_location = glGetAttribLocation(prog, name.c_str());
	if (attrib_location == -1) {
		printf("Unexpected vbo column name.  Got: %s\n", name.c_str());
		piglit_report_result(PIGLIT_FAIL);
	}
	this->index = attrib_location;
	/* If the type is integral, verify that integer vertex
	 * attribute support is present.  Note: we treat it as a FAIL
	 * if support is not present, because it's up to the test to
	 * either (a) not require integer vertex attribute support, or
	 * (b) skip itself if integer vertex attribute support is not
	 * present.
	 */
	if (this->data_type != GL_FLOAT &&
	    (piglit_is_gles() ||
	    (piglit_get_gl_version() < 30 &&
		!piglit_is_extension_supported("GL_EXT_gpu_shader4")))) {
		printf("Test uses glVertexAttribIPointer(),"
		       " which is unsupported.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (this->count < 1 || this->count > 4) {
		printf("Count must be between 1 and 4.  Got: %lu\n", (unsigned long) count);
		piglit_report_result(PIGLIT_FAIL);
	}
}


/**
 * Parse a single number (floating point or integral) from one of the
 * data rows, and store it in the location pointed to by \c data.
 * Update \c text to point to the next character of input.
 *
 * If there is a parse failure, print a description of the problem and
 * then return false.  Otherwise return true.
 */
bool
vertex_attrib_description::parse_datum(const char **text, void *data) const
{
	char *endptr;
	errno = 0;
	switch (this->data_type) {
	case GL_FLOAT: {
		float value = strtof_hex(*text, &endptr);
		if (errno == ERANGE) {
			printf("Could not parse as float\n");
			return false;
		}
		*((GLfloat *) data) = value;
		break;
	}
	case GL_DOUBLE: {
		double value = strtod_hex(*text, &endptr);
		if (errno == ERANGE) {
			printf("Could not parse as double\n");
			return false;
		}
		*((GLdouble *) data) = value;
		break;
	}
	case GL_INT: {
		long long value = strtoll(*text, &endptr, 0);
		if (errno == ERANGE) {
			printf("Could not parse as signed integer\n");
			return false;
		}
		*((GLint *) data) = (GLint) value;
		break;
	}
	case GL_UNSIGNED_INT: {
		unsigned long value = strtoul(*text, &endptr, 0);
		if (errno == ERANGE) {
			printf("Could not parse as unsigned integer\n");
			return false;
		}
		*((GLuint *) data) = (GLuint) value;
		break;
	}
	default:
		assert(!"Unexpected data type");
		endptr = NULL;
		break;
	}
	*text = endptr;
	return true;
}


/**
 * Execute the necessary GL calls to bind this attribute to its data.
 */
void
vertex_attrib_description::setup(size_t *offset, size_t stride) const
{
	int attribute_size = ATTRIBUTE_SIZE;
	GLuint actual_index = this->index + this->matrix_index
		+ this->array_index * this->data_type_cols;
	switch (this->data_type) {
	case GL_FLOAT:
		glVertexAttribPointer(actual_index, this->count,
				      this->data_type, GL_FALSE, stride,
				      (void *) *offset);
		break;
	case GL_DOUBLE:
		if (!piglit_is_extension_supported("GL_ARB_vertex_attrib_64bit")) {
			fprintf(stderr,"vertex_attrib_description fail. no 64-bit float support\n");
			return;
		}
		glVertexAttribLPointer(actual_index, this->count,
				       this->data_type, stride,
				       (void *) *offset);
		attribute_size *= 2;
		break;
	default:
		if (piglit_is_gles() && piglit_get_gl_version() < 30) {
			fprintf(stderr,"vertex_attrib_description fail. no int support\n");
			return;
		}
		glVertexAttribIPointer(actual_index, this->count,
				       this->data_type, stride,
				       (void *) *offset);
		break;
	}
	glEnableVertexAttribArray(actual_index);
	*offset += attribute_size * this->count;
}


/**
 * Data structure containing all of the data parsed from the text
 * input, as well as the methods that parse it and convert it to GL
 * calls.
 */
class vbo_data
{
public:
	vbo_data(std::string const &text, GLuint prog);
	size_t setup() const;

private:
	void parse_header_line(const std::string &line, GLuint prog);
	void parse_data_line(const std::string &line, unsigned int line_num);
	void parse_line(std::string line, unsigned int line_num, GLuint prog);

	/**
	 * True if the header line has already been parsed.
	 */
	bool header_seen;

	/**
	 * Description of each attribute.
	 */
	std::vector<vertex_attrib_description> attribs;

	/**
	 * Raw data buffer containing parsed numbers.
	 */
	std::vector<char> raw_data;

	/**
	 * Number of bytes in each row of raw_data.
	 */
	size_t stride;

	/**
	 * Number of rows in raw_data.
	 */
	size_t num_rows;
};



static bool
is_blank_line(const std::string &line)
{
	for (size_t i = 0; i < line.size(); ++i) {
		if (!isspace(line[i]))
			return false;
	}
	return true;
}


/**
 * Populate this->attribs and compute this->stride based on column
 * headers.
 *
 * If there is a parse failure, print a description of the problem and
 * then exit with PIGLIT_FAIL.
 */
void
vbo_data::parse_header_line(const std::string &line, GLuint prog)
{
	size_t pos = 0;
	this->stride = 0;
	while (pos < line.size()) {
		if (isspace(line[pos])) {
			++pos;
		} else {
			size_t column_header_end = pos;
			int mul;
			while (column_header_end < line.size() &&
			       !isspace(line[column_header_end]))
				++column_header_end;
			std::string column_header = line.substr(
				pos, column_header_end - pos);
			vertex_attrib_description desc(
				prog, column_header.c_str());
			attribs.push_back(desc);
			mul = (desc.data_type == GL_DOUBLE) ? 2 : 1;
			this->stride += ATTRIBUTE_SIZE * desc.count * mul;
			pos = column_header_end + 1;
		}
	}
}


/**
 * Convert a data row into binary form and append it to this->raw_data.
 *
 * If there is a parse failure, print a description of the problem and
 * then exit with PIGLIT_FAIL.
 */
void
vbo_data::parse_data_line(const std::string &line, unsigned int line_num)
{
	/* Allocate space in raw_data for this line */
	size_t old_size = this->raw_data.size();
	this->raw_data.resize(old_size + this->stride);
	char *data_ptr = &this->raw_data[old_size];

	const char *line_ptr = line.c_str();
	for (size_t i = 0; i < this->attribs.size(); ++i) {
		for (size_t j = 0; j < this->attribs[i].count; ++j) {
			int mul = (this->attribs[i].data_type == GL_DOUBLE) ? 2 : 1;
			if (!this->attribs[i].parse_datum(&line_ptr,
							  data_ptr)) {
				printf("At line %u of [vertex data] section\n",
				       line_num);
				printf("Offending text: %s\n", line_ptr);
				piglit_report_result(PIGLIT_FAIL);
			}
			data_ptr += ATTRIBUTE_SIZE * mul;
		}
	}

	++this->num_rows;
}


/**
 * Parse a line of input text.
 *
 * If there is a parse failure, print a description of the problem and
 * then exit with PIGLIT_FAIL.
 */
void
vbo_data::parse_line(std::string line, unsigned int line_num, GLuint prog)
{
	/* Ignore end-of-line comments */
	line = line.substr(0, line.find('#'));

	/* Ignore blank or comment-only lines */
	if (is_blank_line(line))
		return;

	if (!this->header_seen) {
		this->header_seen = true;
		parse_header_line(line, prog);
	} else {
		parse_data_line(line, line_num);
	}
}


/**
 * Parse the input but don't execute any GL commands.
 *
 * If there is a parse failure, print a description of the problem and
 * then exit with PIGLIT_FAIL.
 */
vbo_data::vbo_data(const std::string &text, GLuint prog)
	: header_seen(false), stride(0), num_rows(0)
{
	unsigned int line_num = 1;

	size_t pos = 0;
	while (pos < text.size()) {
		size_t end_of_line = text.find('\n', pos);
		if (end_of_line == std::string::npos)
			end_of_line = text.size();
		parse_line(text.substr(pos, end_of_line), line_num++, prog);
		pos = end_of_line + 1;
	}
}


/**
 * Execute the necessary GL commands to set up the vertex data passed
 * to the constructor.
 */
size_t
vbo_data::setup() const
{
	GLuint buffer_handle;
	glGenBuffers(1, &buffer_handle);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_handle);
	glBufferData(GL_ARRAY_BUFFER, this->stride * this->num_rows,
		     &this->raw_data[0], GL_STATIC_DRAW);

	size_t offset = 0;
	for (size_t i = 0; i < attribs.size(); ++i)
		attribs[i].setup(&offset, this->stride);

	/* Leave buffer bound for later draw calls */

	return this->num_rows;
}


/**
 * Set up a vertex buffer object for the program prog based on the
 * data encoded in text_start.  text_end indicates the end of the text
 * string; if it is NULL, the string is assumed to be null-terminated.
 *
 * Return value is the number of rows of vertex data found.
 *
 * For details about the format of the text string, see the comment at
 * the top of this file.
 */
size_t
setup_vbo_from_text(GLuint prog, const char *text_start, const char *text_end)
{
	if (text_end == NULL)
		text_end = text_start + strlen(text_start);
	std::string text(text_start, text_end);
	return vbo_data(text, prog).setup();
}
