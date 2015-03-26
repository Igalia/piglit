/*
 * Copyright © 2015 Intel Corporation
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

/**
 * \file getprogramresourceiv.c
 *
 * Tests the values returned by GetProgramResourceiv on multiple pipelines.
 * Also checks some error cases.
 *
 * From the GL_ARB_program_interface_query spec:
 *  "The command
 *
 *  void GetProgramResourceiv(uint program, enum programInterface,
 *                            uint index, sizei propCount,
 *                            const enum *props, sizei bufSize,
 *                            sizei *length, int *params);
 *
 *  returns values for multiple properties of a single active resource with an
 *  index of <index> in the interface <programInterface> of program object
 *  <program>.  For each resource, values for <propCount> properties specified
 *  by the array <props> are returned.  The error INVALID_VALUE is generated
 *  if <propCount> is zero.  The error INVALID_ENUM is generated if any value
 *  in <props> is not one of the properties described immediately below.  The
 *  error INVALID_OPERATION is generated if any value in <props> is not
 *  allowed for <programInterface>.  The set of allowed <programInterface>
 *  values for each property can be found in Table X.1.

 *  The values associated with the properties of the active resource are
 *  written to consecutive entries in <params>, in increasing order according
 *  to position in <props>.  If no error is generated, only the first
 *  <bufSize> integer values will be written to <params>; any extra values
 *  will not be returned.  If <length> is not NULL, the actual number of
 *  integer values written to <params> will be written to <length>.
 *
 *    Property                     Supported Interfaces
 *    ---------------------------  ----------------------------------------
 *    NAME_LENGTH                  all but ATOMIC_COUNTER_BUFFER
 *
 *    TYPE                         UNIFORM, PROGRAM_INPUT, PROGRAM_OUTPUT,
 *                                   TRANSFORM_FEEDBACK_VARYING,
 *                                   BUFFER_VARIABLE
 *
 *    ARRAY_SIZE                   UNIFORM, BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT, VERTEX_SUBROUTINE_
 *                                   UNIFORM, TESS_CONTROL_SUBROUTINE_UNIFORM,
 *                                   TESS_EVALUATION_SUBROUTINE_UNIFORM,
 *                                   GEOMETRY_SUBROUTINE_UNIFORM, FRAGMENT_
 *                                   SUBROUTINE_UNIFORM, COMPUTE_SUBROUTINE_
 *                                   UNIFORM, TRANSFORM_FEEDBACK_VARYING
 *
 *    OFFSET                       UNIFORM, BUFFER_VARIABLE
 *    BLOCK_INDEX                  UNIFORM, BUFFER_VARIABLE
 *    ARRAY_STRIDE                 UNIFORM, BUFFER_VARIABLE
 *    MATRIX_STRIDE                UNIFORM, BUFFER_VARIABLE
 *    IS_ROW_MAJOR                 UNIFORM, BUFFER_VARIABLE
 *
 *    ATOMIC_COUNTER_BUFFER_INDEX  UNIFORM
 *
 *    BUFFER_BINDING               UNIFORM_BLOCK, ATOMIC_COUNTER_BUFFER,
 *                                   SHADER_STORAGE_BLOCK
 *    BUFFER_DATA_SIZE             UNIFORM_BLOCK, ATOMIC_COUNTER_BUFFER
 *                                   SHADER_STORAGE_BLOCK
 *    NUM_ACTIVE_VARIABLES         UNIFORM_BLOCK, ATOMIC_COUNTER_BUFFER
 *                                   SHADER_STORAGE_BLOCK
 *    ACTIVE_VARIABLES             UNIFORM_BLOCK, ATOMIC_COUNTER_BUFFER
 *                                   SHADER_STORAGE_BLOCK
 *
 *    REFERENCED_BY_VERTEX_        UNIFORM, UNIFORM_BLOCK, ATOMIC_COUNTER_
 *      SHADER                       BUFFER, SHADER_STORAGE_BLOCK,
 *                                   BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT
 *
 *    REFERENCED_BY_TESS_          UNIFORM, UNIFORM_BLOCK, ATOMIC_COUNTER_
 *      CONTROL_SHADER               BUFFER, SHADER_STORAGE_BLOCK,
 *                                   BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT
 *
 *    REFERENCED_BY_TESS_          UNIFORM, UNIFORM_BLOCK, ATOMIC_COUNTER_
 *      EVALUATION_SHADER            BUFFER, SHADER_STORAGE_BLOCK,
 *                                   BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT
 *
 *    REFERENCED_BY_GEOMETRY_      UNIFORM, UNIFORM_BLOCK, ATOMIC_COUNTER_
 *      SHADER                       BUFFER, SHADER_STORAGE_BLOCK,
 *                                   BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT
 *
 *    REFERENCED_BY_FRAGMENT_      UNIFORM, UNIFORM_BLOCK, ATOMIC_COUNTER_
 *      SHADER                       BUFFER, SHADER_STORAGE_BLOCK,
 *                                   BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT
 *
 *    REFERENCED_BY_COMPUTE_       UNIFORM, UNIFORM_BLOCK, ATOMIC_COUNTER_
 *      SHADER                       BUFFER, SHADER_STORAGE_BLOCK,
 *                                   BUFFER_VARIABLE, PROGRAM_INPUT,
 *                                   PROGRAM_OUTPUT
 *
 *    NUM_COMPATIBLE_SUBROUTINES   VERTEX_SUBROUTINE_UNIFORM, TESS_CONTROL_
 *                                   SUBROUTINE_UNIFORM, TESS_EVALUATION_
 *                                   SUBROUTINE_UNIFORM, GEOMETRY_SUBROUTINE_
 *                                   UNIFORM, FRAGMENT_SUBROUTINE_UNIFORM,
 *                                   COMPUTE_SUBROUTINE_UNIFORM
 *
 *    COMPATIBLE_SUBROUTINES       VERTEX_SUBROUTINE_UNIFORM, TESS_CONTROL_
 *                                   SUBROUTINE_UNIFORM, TESS_EVALUATION_
 *                                   SUBROUTINE_UNIFORM, GEOMETRY_SUBROUTINE_
 *                                   UNIFORM, FRAGMENT_SUBROUTINE_UNIFORM,
 *                                   COMPUTE_SUBROUTINE_UNIFORM
 *
 *    TOP_LEVEL_ARRAY_SIZE         BUFFER_VARIABLE
 *
 *    TOP_LEVEL_ARRAY_STRIDE       BUFFER_VARIABLE
 *
 *    LOCATION                     UNIFORM, PROGRAM_INPUT, PROGRAM_OUTPUT,
 *                                   VERTEX_SUBROUTINE_UNIFORM, TESS_CONTROL_
 *                                   SUBROUTINE_UNIFORM, TESS_EVALUATION_
 *                                   SUBROUTINE_UNIFORM, GEOMETRY_SUBROUTINE_
 *                                   UNIFORM, FRAGMENT_SUBROUTINE_UNIFORM,
 *                                   COMPUTE_SUBROUTINE_UNIFORM
 *
 *    LOCATION_INDEX               PROGRAM_OUTPUT
 *
 *    IS_PER_PATCH                 PROGRAM_INPUT, PROGRAM_OUTPUT
 *
 *    Table X.1, GetProgramResourceiv properties and supported interfaces
 *
 *  For the property NAME_LENGTH, a single integer identifying the length of
 *  the name string associated with an active variable, interface block, or
 *  subroutine is written to <params>.  The name length includes a terminating
 *  null character.
 *
 *  For the property TYPE, a single integer identifying the type of an active
 *  variable is written to <params>.  The integer returned is one of the
 *  values found in table 2.16.
 *
 *  For the property ARRAY_SIZE, a single integer identifying the number of
 *  active array elements of an active variable is written to <params>. The
 *  array size returned is in units of the type associated with the property
 *  TYPE. For active variables not corresponding to an array of basic types,
 *  the value one is written to <params>. If the variable is a shader
 *  storage block member in an array with no declared size, the value zero
 *  is written to <params>.
 *
 *  For the property BLOCK_INDEX, a single integer identifying the index of
 *  the active interface block containing an active variable is written to
 *  <params>.  If the variable is not the member of an interface block, the
 *  value -1 is written to <params>.
 *
 *  For the property OFFSET, a single integer identifying the offset of an
 *  active variable is written to <params>.  For active variables backed by a
 *  buffer object, the value written is the offset, in basic machine units,
 *  relative to the base of buffer range holding the values of the variable.
 *  For active variables not backed by a buffer object, an offset of -1 is
 *  written to <params>.
 *
 *  For the property ARRAY_STRIDE, a single integer identifying the stride
 *  between array elements in an active variable is written to <params>.  For
 *  active variables declared as an array of basic types, the value written is
 *  the difference, in basic machine units, between the offsets of consecutive
 *  elements in an array.  For active variables not declared as an array of
 *  basic types, zero is written to <params>.  For active variables not backed
 *  by a buffer object, -1 is written to <params>, regardless of the variable
 *  type.
 *
 *  For the property MATRIX_STRIDE, a single integer identifying the stride
 *  between columns of a column-major matrix or rows of a row-major matrix is
 *  written to <params>.  For active variables declared a single matrix or
 *  array of matrices, the value written is the difference, in basic machine
 *  units, between the offsets of consecutive columns or rows in each matrix.
 *  For active variables not declared as a matrix or array of matrices, zero
 *  is written to <params>.  For active variables not backed by a buffer
 *  object, -1 is written to <params>, regardless of the variable type.
 *
 *  For the property IS_ROW_MAJOR, a single integer identifying whether an
 *  active variable is a row-major matrix is written to <params>.  For active
 *  variables backed by a buffer object, declared as a single matrix or array
 *  of matrices, and stored in row-major order, one is written to <params>.
 *  For all other active variables, zero is written to <params>.
 *
 *  For the property ATOMIC_COUNTER_BUFFER_INDEX, a single integer identifying
 *  the index of the active atomic counter buffer containing an active
 *  variable is written to <params>.  If the variable is not an atomic counter
 *  uniform, the value -1 is written to <params>.
 *
 *  For the property of BUFFER_BINDING, to index of the buffer binding point
 *  associated with the active uniform block, shader storage block, or atomic
 *  counter buffer is written to <params>.
 *
 *  For the property of BUFFER_DATA_SIZE, then the implementation-dependent
 *  minimum total buffer object size, in basic machine units, required to hold
 *  all active variables associated with an active uniform block, shader
 *  storage block, or atomic counter buffer is written to <params>.  If the
 *  final member of an active shader storage block is array with no declared
 *  size, the minimum buffer size is computed assuming the array was declared
 *  as an array with one element.
 *
 *  For the property of NUM_ACTIVE_VARIABLES, the number of active variables
 *  associated with an active uniform block, shader storage block, or atomic
 *  counter buffer is written to <params>.
 *
 *  For the property of ACTIVE_VARIABLES, an array of active variable indices
 *  associated with an active uniform block, shader storage block, or atomic
 *  counter buffer is written to <params>.  The number of values written to
 *  <params> for an active resource is given by the value of the property
 *  NUM_ACTIVE_VARIABLES for the resource.
 *
 *  For the properties REFERENCED_BY_VERTEX_SHADER,
 *  REFERENCED_BY_TESS_CONTROL_SHADER, REFERENCED_BY_TESS_EVALUATION_SHADER,
 *  REFERENCED_BY_GEOMETRY_SHADER, REFERENCED_BY_FRAGMENT_SHADER, and
 *  REFERENCED_BY_COMPUTE_SHADER, a single integer is written to <params>,
 *  identifying whether the active resource is referenced by the vertex,
 *  tessellation control, tessellation evaluation, geometry, or fragment
 *  shaders, respectively, in the program object.  The value one is written to
 *  <params> if an active variable is referenced by the corresponding shader,
 *  or if an active uniform block, shader storage block, or atomic counter
 *  buffer contains at least one variable referenced by the corresponding
 *  shader.  Otherwise, the value zero is written to <params>.
 *
 *  For the property TOP_LEVEL_ARRAY_SIZE, a single integer identifying the
 *  number of active array elements of the top-level shader storage block
 *  member containing to the active variable is written to <params>.  If the
 *  top-level block member is not declared as an array, the value one is
 *  written to <params>.  If the top-level block member is an array with no
 *  declared size, the value zero is written to <params>.
 *
 *  For the property TOP_LEVEL_ARRAY_STRIDE, a single integer identifying the
 *  stride between array elements of the top-level shader storage block member
 *  containing the active variable is written to <params>.  For top-level
 *  block members declared as arrays, the value written is the difference, in
 *  basic machine units, between the offsets of the active variable for
 *  consecutive elements in the top-level array.  For top-level block members
 *  not declared as an array, zero is written to <params>.
 *
 *  For the property LOCATION, a single integer identifying the assigned
 *  location for an active uniform, input, output, or subroutine uniform
 *  variable is written to <params>.  For input, output, or uniform variables
 *  with locations specified by a layout qualifier, the specified location is
 *  used.  For vertex shader input or fragment shader output variables without
 *  a layout qualifier, the location assigned when a program is linked is
 *  written to <params>.  For all other input and output variables, the value
 *  -1 is written to <params>.  For uniforms in uniform blocks, the value -1
 *  is written to <params>.
 *
 *  For the property LOCATION_INDEX, a single integer identifying the fragment
 *  color index of an active fragment shader output variable is written to
 *  <params>.  If the active variable is an output for a non-fragment shader,
 *  the value -1 will be written to <params>.
 *
 *  For the property IS_PER_PATCH, a single integer identifying whether the
 *  input or output is a per-patch attribute.  If the active variable is a
 *  per-patch attribute (declared with the "patch" qualifier), the value one
 *  is written to <params>; otherwise, the value zero is written to <params>."
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

GLuint prog_std = -1; /* (vs,gs,fs)_std */
GLuint prog_stor = -1; /* (vs,gs,fs)_stor */
GLuint prog_sub = -1; /* (vs,gs,fs)_sub */
GLuint prog_sub_tess = -1; /* tcs_sub */
GLuint prog_cs = -1; /* cs_sub */
GLuint prog_loc = -1; /* (vs,fs)_loc */
GLuint prog_atom = -1; /* fs_atom */

struct subtest_t {
	GLuint *prog;
	GLenum programInterface;
	const char *name;
	void *inputs;

	/* set to -1 to disable the test */
	struct check_t{
		GLenum prop;
		size_t count;
		int values[10];
	} props[25];
};

const char *fs_std_fs_uniform_blk[] = {"fs_color", "fs_array[0]", NULL };
const char *fs_stor_gs_buf_blk[] = {"gs_buf_var", NULL };
const char *vs_sub_uniforms[] = {"vss", "vss2", NULL };
const char *tess_sub_uniforms[] = {"tcss", NULL };
const char *cs_sub_uniforms[] = {"css", NULL };

static const struct subtest_t subtests[] = {
 { &prog_std, GL_PROGRAM_INPUT, "vs_input0", NULL, {
	{ GL_NAME_LENGTH, 1, { 10 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_IS_PER_PATCH, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_std, GL_PROGRAM_OUTPUT, "fs_output0", NULL, {
	{ GL_NAME_LENGTH, 1, { 11 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_LOCATION_INDEX, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_IS_PER_PATCH, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_std, GL_UNIFORM, "vs_test", "vs_uniform_block", {
	{ GL_NAME_LENGTH, 1, { 8 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_OFFSET, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_BLOCK_INDEX, 1, { 1 } }, /* compared to vs_uniform_block's idx */
	{ GL_ARRAY_STRIDE, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_MATRIX_STRIDE, 1, { 0 } },
	{ GL_IS_ROW_MAJOR, 1, { 0 } },
	{ GL_ATOMIC_COUNTER_BUFFER_INDEX, 1, { -1 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { -1 } }, /* valid index == anything but -1 */
	{ 0, 0, { 0 } }}
 },
 { &prog_loc, GL_PROGRAM_INPUT, "input0", NULL, {
	{ GL_NAME_LENGTH, 1, { 7 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { 3 } }, /* value checked because it uses prog_loc */
	{ GL_IS_PER_PATCH, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_loc, GL_PROGRAM_OUTPUT, "output0", NULL, {
	{ GL_NAME_LENGTH, 1, { 8 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { 1 } }, /* value checked because it uses prog_loc */
	{ GL_LOCATION_INDEX, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_IS_PER_PATCH, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_loc, GL_UNIFORM, "color", NULL, {
	{ GL_NAME_LENGTH, 1, { 6 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_OFFSET, 1, { -1 } }, /* valid index == anything but -1 */
	{ GL_BLOCK_INDEX, 1, { -1 } }, /* invalid index */
	{ GL_ARRAY_STRIDE, 1, { -1 } }, /* valid index == anything but -1 */
	{ GL_MATRIX_STRIDE, 1, { -1 } },
	{ GL_IS_ROW_MAJOR, 1, { 0 } },
	{ GL_ATOMIC_COUNTER_BUFFER_INDEX, 1, { -1 } }, /* valid index == anything but -1 */
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { 9 } }, /* valid index == anything but -1 */
	{ 0, 0, { 0 } }}
 },
{ &prog_sub_tess, GL_PROGRAM_OUTPUT, "tcs_patch", NULL, {
       { GL_NAME_LENGTH, 1, { 10 } },
       { GL_TYPE, 1, { GL_FLOAT_VEC4 } },
       { GL_ARRAY_SIZE, 1, { 1 } },
       { GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
       { GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 1 } },
       { GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
       { GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
       { GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 0 } },
       { GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
       { GL_LOCATION, 1, { 0 } }, /* valid index == anything but -1 */
       { GL_LOCATION_INDEX, 1, { -1 } }, /* valid index == anything but -1 */
       { GL_IS_PER_PATCH, 1, { 1 } },
       { 0, 0, { 0 } }}
},
 { &prog_std, GL_UNIFORM, "fs_array", "fs_uniform_block", {
	{ GL_NAME_LENGTH, 1, { 12 } },
	{ GL_TYPE, 1, { GL_FLOAT } },
	{ GL_ARRAY_SIZE, 1, { 4 } },
	{ GL_OFFSET, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_BLOCK_INDEX, 1, { 1 } }, /* compared to fs_uniform_block's idx */
	{ GL_ARRAY_STRIDE, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_MATRIX_STRIDE, 1, { 0 } },
	{ GL_IS_ROW_MAJOR, 1, { 0 } },
	{ GL_ATOMIC_COUNTER_BUFFER_INDEX, 1, { -1 } }, /* valid index == anything but -1 */
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_LOCATION, 1, { -1 } }, /* valid index == anything but -1 */
	{ 0, 0, { 0 } }}
 },
 { &prog_std, GL_UNIFORM_BLOCK, "fs_uniform_block", fs_std_fs_uniform_blk, {
	{ GL_NAME_LENGTH, 1, { 17 } },
	{ GL_BUFFER_BINDING, 1, { 0 } },
	{ GL_BUFFER_DATA_SIZE, 1, { 32 } }, /* only checks for GL errors */
	{ GL_NUM_ACTIVE_VARIABLES, 1, { 2 } },
	{ GL_ACTIVE_VARIABLES, 2, { 0, 0 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_stor, GL_BUFFER_VARIABLE, "gs_buf_var", "gs_buffer_block", {
	{ GL_NAME_LENGTH, 1, { 11 } },
	{ GL_TYPE, 1, { GL_FLOAT_VEC4 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ GL_OFFSET, 1, { 0 } }, /* valid index == anything but -1 */
	{ GL_BLOCK_INDEX, 1, { 1 } }, /* compared to gs_buffer_block's idx */
	{ GL_ARRAY_STRIDE, 1, { 0 } },  /* valid index == anything but -1 */
	{ GL_MATRIX_STRIDE, 1, { 0 } },
	{ GL_IS_ROW_MAJOR, 1, { 0 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ GL_TOP_LEVEL_ARRAY_SIZE, 1, { 1 } },
	{ GL_TOP_LEVEL_ARRAY_STRIDE, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_stor, GL_SHADER_STORAGE_BLOCK, "gs_buffer_block", fs_stor_gs_buf_blk, {
	{ GL_NAME_LENGTH, 1, { 16 } },
	{ GL_BUFFER_BINDING, 1, { 0 } },
	{ GL_BUFFER_DATA_SIZE, 1, { 16 } }, /* only checks for GL errors */
	{ GL_NUM_ACTIVE_VARIABLES, 1, { 1 } },
	{ GL_ACTIVE_VARIABLES, 1, { 1 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_std, GL_TRANSFORM_FEEDBACK_VARYING, "gs_output0", NULL, {
	{ GL_NAME_LENGTH, 1, { 11 } },
	{ GL_ARRAY_SIZE, 1, { 1 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_sub, GL_VERTEX_SUBROUTINE_UNIFORM, "VERTEX", vs_sub_uniforms, {
	{ GL_NAME_LENGTH, 1, { 7 } },
	{ GL_NUM_COMPATIBLE_SUBROUTINES, 1, { 2 } },
	{ GL_COMPATIBLE_SUBROUTINES, 2, { 0, 1 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_sub_tess, GL_TESS_CONTROL_SUBROUTINE_UNIFORM, "TESS_CONTROL",
   tess_sub_uniforms, {
	{ GL_NAME_LENGTH, 1, { 13 } },
	{ GL_NUM_COMPATIBLE_SUBROUTINES, 1, { 1 } },
	{ GL_COMPATIBLE_SUBROUTINES, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_cs, GL_COMPUTE_SUBROUTINE_UNIFORM, "COMPUTE", cs_sub_uniforms, {
	{ GL_NAME_LENGTH, 1, { 8 } },
	{ GL_NUM_COMPATIBLE_SUBROUTINES, 1, { 1 } },
	{ GL_COMPATIBLE_SUBROUTINES, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
 { &prog_atom, GL_ATOMIC_COUNTER_BUFFER, "fs_counter", NULL, {
	{ GL_BUFFER_BINDING, 1, { 2 } },
	{ GL_BUFFER_DATA_SIZE, 1, { 4 } }, /* only checks for GL errors */
	{ GL_NUM_ACTIVE_VARIABLES, 1, { 1 } },
	{ GL_ACTIVE_VARIABLES, 1, { 0 } },
	{ GL_REFERENCED_BY_VERTEX_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_CONTROL_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_TESS_EVALUATION_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_GEOMETRY_SHADER, 1, { 0 } },
	{ GL_REFERENCED_BY_FRAGMENT_SHADER, 1, { 1 } },
	{ GL_REFERENCED_BY_COMPUTE_SHADER, 1, { 0 } },
	{ 0, 0, { 0 } }}
 },
};

/* WARNING: ATOMIC_COUNTER_BUFFER is left untested because it is impossible to
 * fetch the index of variables which means we cannot reliably test anything
 */

static bool
check_extensions_prop(GLenum prop)
{
	/* First check the availability of the extensions */
	if (prop == GL_ATOMIC_COUNTER_BUFFER_INDEX &&
	    !piglit_is_extension_supported("GL_ARB_shader_atomic_counters")) {
		return false;
	}

	if ((prop == GL_TOP_LEVEL_ARRAY_SIZE ||
	    prop == GL_TOP_LEVEL_ARRAY_STRIDE) &&
	    !piglit_is_extension_supported("GL_ARB_shader_storage_buffer_object")) {
		return false;
	}

	if ((prop == GL_NUM_COMPATIBLE_SUBROUTINES ||
	     prop == GL_COMPATIBLE_SUBROUTINES) &&
	     !piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		 return false;
	}

	if ((prop == GL_REFERENCED_BY_TESS_CONTROL_SHADER ||
	     prop == GL_REFERENCED_BY_TESS_EVALUATION_SHADER) &&
	     !piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
		 return false;
	}

	if ((prop == GL_REFERENCED_BY_COMPUTE_SHADER ||
	     prop == GL_COMPUTE_SUBROUTINE_UNIFORM ||
	     prop == GL_IS_PER_PATCH) &&
	     !piglit_is_extension_supported("GL_ARB_compute_shader") &&
	     !piglit_is_extension_supported("GL_ARB_shader_image_load_store")) {
		 return false;
	}

	return true;
}

static bool
is_resource_in_list(const char **list, const char *resource, int index,
		    bool check_order)
{
	int i = 0;
	while (list && list[i]) {
		if (strcmp(list[i], resource) == 0) {
			return !check_order || index == i;
		}
		i++;
	}

	return false;
}

static void
basic_check(const char *subsubtest, int value, int expected_value, bool *pass)
{
	/* no real check can be done other than checking that
	 * the index or offset is not invalid when it was
	 * supposed to be valid (or the other way round).
	 */
	if ((value >= 0  && expected_value == -1) ||
	    (value == -1 && expected_value >= 0)) {
		const char *validity = expected_value == -1 ?
				       "an invalid" : "a valid";
		fprintf(stderr, "'%s' expected %s offset or index but "
				"got %i\n", subsubtest, validity, value);
		*pass = false;
	}
}

static void
check_prop(GLuint prog, GLenum programInterface, int index, const char *name,
	   void *inputs, struct check_t c, bool *pass)
{
	int values[10], parent_idx, i;
	char subsubtest[150];
	GLsizei length, tmp = -1;
	char buf[21];
	GLenum pif;
	GLuint loc;

	/* skip the test if it is not supported */
	if(!check_extensions_prop(c.prop)) {
		return;
	}

	/* generate the name of the subsubtest for error-reporting purposes */
	snprintf(subsubtest, sizeof(subsubtest), "%s: %s on %s",
		 name, piglit_get_gl_enum_name(c.prop),
		 piglit_get_gl_enum_name(programInterface));

	/* retrieve the property */
	glGetProgramResourceiv(prog, programInterface, index, 1, &c.prop, 10,
			       &length, values);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("	Latest error generated while running '%s'\n",
		       subsubtest);
		*pass = false;
		return;
	}

	/* check the return value */
	switch (c.prop) {
	case GL_OFFSET:
	case GL_ARRAY_STRIDE:
	case GL_ATOMIC_COUNTER_BUFFER_INDEX:
		basic_check(subsubtest, values[0], c.values[0], pass);
		break;

	case GL_BLOCK_INDEX:
		/* check that the index of the parent matches the name
		 * of the parent
		 */
		switch (programInterface) {
		case GL_UNIFORM:
			pif = GL_UNIFORM_BLOCK;
			break;
		case GL_BUFFER_VARIABLE:
			pif = GL_SHADER_STORAGE_BLOCK;
			break;
		}

		parent_idx = glGetProgramResourceIndex(prog, pif,
						       (char*)inputs);
		piglit_check_gl_error(GL_NO_ERROR);
		if (parent_idx != values[0]) {
			glGetProgramResourceName(prog, programInterface,
						 values[0], sizeof(buf), NULL,
						 buf);

			fprintf(stderr, "'%s' expected parent name to be %s"
				"(idx = %i) but got parent name %s(idx = %i)\n",
				subsubtest, (char*)inputs, parent_idx, buf,
				values[0]);
			*pass = false;
		}
		break;

	case GL_BUFFER_BINDING:
		if (values[0] < 0) {
			fprintf(stderr, "'%s' invalid buffer binding point\n",
				subsubtest);
			*pass = false;
		}

		/* Binding index is necessary for ATOMIC_COUNTER_BUFFER */
		if (programInterface == GL_ATOMIC_COUNTER_BUFFER) {
			if (values[0] != c.values[0]) {
				fprintf(stderr, "'%s' expected binding point %i"
						" but got %i\n", subsubtest,
					c.values[0], values[0]);
				*pass = false;
			}
			break;
		}

		/* check against another API call */
		if (programInterface != GL_UNIFORM_BLOCK) {
			break;
		}

		glGetActiveUniformBlockiv(prog, index, GL_UNIFORM_BLOCK_BINDING,
					  &tmp);
		piglit_check_gl_error(GL_NO_ERROR);
		if (tmp != values[0]) {
			fprintf(stderr, "'%s' inconsistent buffer binding point"
					"(%i) with glGetActiveUniformBlockiv"
					"(%i)\n", subsubtest, values[0], tmp);
			*pass = false;
		}

		break;

	case GL_ACTIVE_VARIABLES:
	case GL_COMPATIBLE_SUBROUTINES:
		switch (programInterface) {
		case GL_UNIFORM_BLOCK:
			pif = GL_UNIFORM;
			break;
		case GL_SHADER_STORAGE_BLOCK:
			pif = GL_BUFFER_VARIABLE;
			break;
		case GL_VERTEX_SUBROUTINE_UNIFORM:
			pif = GL_VERTEX_SUBROUTINE;
			break;
		case GL_TESS_CONTROL_SUBROUTINE_UNIFORM:
			pif = GL_TESS_CONTROL_SUBROUTINE;
			break;
		case GL_COMPUTE_SUBROUTINE_UNIFORM:
			pif = GL_COMPUTE_SUBROUTINE;
			break;
		}

		/* check that the return count is as expected */
		if (c.count != length) {
			fprintf(stderr, "'%s' expected %zu entries but got %i"
					"\n", subsubtest, c.count, length);
			length = 1;
			*pass = false;
			break;
		}

		/* harcode the index test for GL_ATOMIC_COUNTER_BUFFER */
		if (programInterface == GL_ATOMIC_COUNTER_BUFFER) {

			if (values[0] != 0) {
				fprintf(stderr, "'%s' expected index 0 but got "
						"%i", subsubtest, values[0]);
				*pass = false;
			}
			break;
		}

		for (i = 0; i < length; i++) {
			buf[0] = '\0';
			glGetProgramResourceName(prog, pif, values[i],
						 sizeof(buf), NULL, buf);
			piglit_check_gl_error(GL_NO_ERROR);
			if (!is_resource_in_list(inputs, buf, i, false)) {
				fprintf(stderr, "'%s' could not find active "
					"resource '%s' (idx = %i) in the active"
					" list\n", subsubtest, buf, values[i]);
				*pass = false;
			}
		}
		break;

	case GL_BUFFER_DATA_SIZE:
		/* Nothing we can check here... */
		break;

	case GL_LOCATION:
		loc = glGetProgramResourceLocation(prog, programInterface,
						   name);
		piglit_check_gl_error(GL_NO_ERROR);
		if (loc != values[0]) {
			fprintf(stderr, "'%s' inconsistent value between "
					"glGetProgramResourceiv(%i) and "
					"glGetProgramResourceLocation(%i).\n",
				subsubtest, values[0], loc);
			*pass = false;
			break;
		}

		if (prog == prog_loc && values[0] != c.values[0]) {
			fprintf(stderr, "'%s' expected location %i but got "
					"%i\n", subsubtest, c.values[0],
					values[0]);
			*pass = false;
			break;
		}

		/* continue by testing the (in)validity of the index */
		basic_check(subsubtest, values[0], c.values[0], pass);
		break;

	case GL_LOCATION_INDEX:
		loc = glGetProgramResourceLocationIndex(prog, programInterface,
							name);
		piglit_check_gl_error(GL_NO_ERROR);
		if (loc != values[0]) {
			fprintf(stderr, "'%s' inconsistent value between "
					"glGetProgramResourceiv(%i) and "
					"glGetProgramResourceLocationIndex(%i)."
					"\n", subsubtest, values[0], loc);
			*pass = false;
			break;
		}

		/* continue by testing the (in)validity of the index */
		basic_check(subsubtest, values[0], c.values[0], pass);
		break;

	default:
		/* check that the return count is as expected */
		if (c.count != length) {
			fprintf(stderr, "'%s' expected %zu entries but got %i"
					"\n", subsubtest, c.count, length);
			length = 1;
			*pass = false;
			break;
		}

		/* go through all the values returned */
		for (i = 0; i < length; i++) {
			if (values[i] != c.values[i]) {
				fprintf(stderr, "'%s' expected %i but got %i at"
					" index %i\n", subsubtest, c.values[i],
					values[i], i);
				*pass = false;
			}
		}

		break;
	}
}

static bool
check_extensions(GLuint prog, GLenum programInterface)
{
	/* First check the availability of the extensions */
	if ((programInterface == GL_BUFFER_VARIABLE ||
	    programInterface == GL_SHADER_STORAGE_BLOCK ||
	    prog == prog_stor) &&
	    !piglit_is_extension_supported("GL_ARB_shader_storage_buffer_object")) {
		return false;
	}

	if ((programInterface == GL_VERTEX_SUBROUTINE ||
	     programInterface == GL_GEOMETRY_SUBROUTINE ||
	     programInterface == GL_FRAGMENT_SUBROUTINE ||
	     programInterface == GL_COMPUTE_SUBROUTINE ||
	     programInterface == GL_VERTEX_SUBROUTINE_UNIFORM ||
	     programInterface == GL_GEOMETRY_SUBROUTINE_UNIFORM ||
	     programInterface == GL_FRAGMENT_SUBROUTINE_UNIFORM ||
	     programInterface == GL_COMPUTE_SUBROUTINE_UNIFORM ||
	     programInterface == GL_TESS_CONTROL_SUBROUTINE ||
	     programInterface == GL_TESS_EVALUATION_SUBROUTINE ||
	     programInterface == GL_TESS_CONTROL_SUBROUTINE_UNIFORM ||
	     programInterface == GL_TESS_EVALUATION_SUBROUTINE_UNIFORM ||
	     programInterface == GL_COMPUTE_SUBROUTINE_UNIFORM ||
	     prog == prog_sub || prog == prog_sub_tess) &&
	     !piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		 return false;
	}

	if ((programInterface == GL_TESS_CONTROL_SUBROUTINE ||
	     programInterface == GL_TESS_EVALUATION_SUBROUTINE ||
	     programInterface == GL_TESS_CONTROL_SUBROUTINE_UNIFORM ||
	     programInterface == GL_TESS_EVALUATION_SUBROUTINE_UNIFORM ||
	     prog == prog_sub_tess) &&
	     !piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
		 return false;
	}

	if ((programInterface == GL_COMPUTE_SUBROUTINE ||
	     programInterface == GL_COMPUTE_SUBROUTINE_UNIFORM ||
	     prog == prog_cs) &&
	     !piglit_is_extension_supported("GL_ARB_compute_shader") &&
	     !piglit_is_extension_supported("GL_ARB_shader_image_load_store")) {
		 return false;
	}

	return true;
}

static void
run_subtest(const struct subtest_t st, bool *pass)
{
	enum piglit_result result;
	bool local_pass = true;
	int index, i = 0;

	if (*st.prog == -1 || !check_extensions(*st.prog, st.programInterface)) {
		result = PIGLIT_SKIP;
		goto report_result;
	}

	if (st.programInterface != GL_ATOMIC_COUNTER_BUFFER) {
		index = glGetProgramResourceIndex(*st.prog,
						  st.programInterface, st.name);
		piglit_check_gl_error(GL_NO_ERROR);
		if (index < 0) {
			printf("	Could not find resource '%s' in program"
			       " %u\n", st.name, *st.prog);
			result = PIGLIT_FAIL;
			goto report_result;
		}
	} else {
		/* As we cannot query the index of an atomic variable, let's
		 * hardcode it to 0 and make sure the program only has ONE
		 * atomic variable. In our case, we only use the fs_atom stage
		 * which defines only one variable.
		 */
		index = 0;
	}

	while (st.props[i].prop != 0) {
		check_prop(*st.prog, st.programInterface, index, st.name,
			   st.inputs, st.props[i], &local_pass);
		i++;
	}

	*pass = *pass && local_pass;
	result = local_pass ? PIGLIT_PASS : PIGLIT_FAIL;

report_result:
	piglit_report_subtest_result(result, "%s on %s", st.name,
				  piglit_get_gl_enum_name(st.programInterface));
}

void
piglit_init(int argc, char **argv)
{
	static const char *st_r_tf_varying[] = {"gs_output0", NULL};

	piglit_require_extension("GL_ARB_program_interface_query");
	piglit_require_extension("GL_ARB_separate_shader_objects");

	/* Allocate the different programs */
	prog_std = piglit_build_simple_program_unlinked_multiple_shaders(
					GL_VERTEX_SHADER, vs_std,
					GL_GEOMETRY_SHADER, gs_std,
					GL_FRAGMENT_SHADER, fs_std,
					0);
	glTransformFeedbackVaryings(prog_std, 1, st_r_tf_varying,
				    GL_INTERLEAVED_ATTRIBS);
	piglit_check_gl_error(GL_NO_ERROR);

	/* force the compiler not to optimise away inputs/outputs */
	glProgramParameteri(prog_std, GL_PROGRAM_SEPARABLE, GL_TRUE);
	piglit_check_gl_error(GL_NO_ERROR);

	glLinkProgram(prog_std);
	if (!piglit_link_check_status(prog_std)) {
		glDeleteProgram(prog_std);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (piglit_is_extension_supported("GL_ARB_shader_storage_buffer_object")) {
		prog_stor = piglit_build_simple_program_multiple_shaders(
						GL_VERTEX_SHADER, vs_stor,
						GL_GEOMETRY_SHADER, gs_stor,
						GL_FRAGMENT_SHADER, fs_stor,
						0);
		if (!piglit_link_check_status(prog_stor)) {
			glDeleteProgram(prog_stor);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (piglit_is_extension_supported("GL_ARB_explicit_attrib_location") &&
	    piglit_is_extension_supported("GL_ARB_explicit_uniform_location")) {
		prog_loc = piglit_build_simple_program_multiple_shaders(
						GL_VERTEX_SHADER, vs_loc,
						GL_FRAGMENT_SHADER, fs_loc,
						0);
		if (!piglit_link_check_status(prog_loc)) {
			glDeleteProgram(prog_loc);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (piglit_is_extension_supported("GL_ARB_shader_atomic_counters")) {
		prog_atom = piglit_build_simple_program_unlinked_multiple_shaders(
						GL_FRAGMENT_SHADER, fs_atom,
						0);

		/* force the compiler not to optimise away inputs/outputs */
		glProgramParameteri(prog_atom, GL_PROGRAM_SEPARABLE,
				    GL_TRUE);
		piglit_check_gl_error(GL_NO_ERROR);

		glLinkProgram(prog_atom);
		if (!piglit_link_check_status(prog_atom)) {
			glDeleteProgram(prog_atom);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (!piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		return;
	}

	prog_sub = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs_sub,
				GL_GEOMETRY_SHADER, gs_sub,
				GL_FRAGMENT_SHADER, fs_sub,
				0);
	if (!piglit_link_check_status(prog_sub)) {
		glDeleteProgram(prog_sub);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
		prog_sub_tess =
			  piglit_build_simple_program_unlinked_multiple_shaders(
					GL_TESS_CONTROL_SHADER, tcs_sub,
					0);
		/* force the compiler not to optimise away inputs/outputs */
		glProgramParameteri(prog_sub_tess, GL_PROGRAM_SEPARABLE,
				    GL_TRUE);
		piglit_check_gl_error(GL_NO_ERROR);

		glLinkProgram(prog_sub_tess);
		if (!piglit_link_check_status(prog_sub_tess)) {
			glDeleteProgram(prog_sub_tess);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	if (piglit_is_extension_supported("GL_ARB_compute_shader")) {
		prog_cs = piglit_build_simple_program_multiple_shaders(
						GL_COMPUTE_SHADER, cs_sub,
						0);
		if (!piglit_link_check_status(prog_cs)) {
			glDeleteProgram(prog_cs);
			piglit_report_result(PIGLIT_FAIL);
		}
	}
}

static void
test_error_cases(bool *pass)
{
	GLenum props[] = {GL_NAME_LENGTH};
	GLenum props_invalid[] = {GL_NAME_LENGTH, GL_TRUE, GL_TYPE};
	GLenum props_error[] = {GL_NAME_LENGTH, GL_OFFSET, GL_TYPE};
	int values[10];
	GLuint shader;
	bool prg_tst;

	/* test using an unexisting program ID */
	glGetProgramResourceiv(1337, GL_UNIFORM, 0, 1, props, 10, NULL, values);
	prg_tst = piglit_check_gl_error(GL_INVALID_VALUE);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "Invalid program (undefined ID)");

	/* test using a shader ID */
	shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_empty);
	glGetProgramResourceIndex(shader, GL_UNIFORM, "resource");
	prg_tst = piglit_check_gl_error(GL_INVALID_OPERATION);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "Invalid program (call on shader)");

	/* invalid index. This is unspecified but let's check it is consistent
	 * with GetProgramResourceName.
	 */
	glGetProgramResourceiv(prog_std, GL_UNIFORM, 1337, 0, props, 10, NULL,
			       values);
	prg_tst = piglit_check_gl_error(GL_INVALID_VALUE);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "Invalid index");

	/* test propcount == 0 */
	glGetProgramResourceiv(prog_std, GL_UNIFORM, 0, 0, props, 10, NULL,
			       values);
	prg_tst = piglit_check_gl_error(GL_INVALID_VALUE);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "<propcount> == 0");

	/* test propcount < 0 */
	glGetProgramResourceiv(prog_std, GL_UNIFORM, 0, -1, props, 10, NULL,
			       values);
	prg_tst = piglit_check_gl_error(GL_INVALID_VALUE);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "<propcount> < 0");

	/* one invalid property */
	glGetProgramResourceiv(prog_std, GL_UNIFORM, 0, 3, props_invalid, 10,
			       NULL, values);
	prg_tst = piglit_check_gl_error(GL_INVALID_ENUM);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "prop == GL_TRUE");

	/* property not acceptable for one program interface */
	glGetProgramResourceiv(prog_std, GL_PROGRAM_INPUT, 0, 3, props_error,
			       10, NULL, values);
	prg_tst = piglit_check_gl_error(GL_INVALID_OPERATION);
	*pass = *pass && prg_tst;
	piglit_report_subtest_result(prg_tst ? PIGLIT_PASS : PIGLIT_FAIL,
				     "GL_OFFSET on GL_PROGRAM_INPUT");
}

static void
glDeleteProgramSafe(GLuint prog)
{
	if (prog != -1)
		glDeleteProgram(prog);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i;

	test_error_cases(&pass);

	/* run all the getprogramresourceiv tests */
	for (i = 0; i < sizeof(subtests) / sizeof(struct subtest_t); i++) {
		run_subtest(subtests[i], &pass);
	}

	glDeleteProgramSafe(prog_atom);
	glDeleteProgramSafe(prog_loc);
	glDeleteProgramSafe(prog_cs);
	glDeleteProgramSafe(prog_sub_tess);
	glDeleteProgramSafe(prog_sub);
	glDeleteProgramSafe(prog_stor);
	glDeleteProgramSafe(prog_std);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
