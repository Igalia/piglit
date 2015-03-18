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
 * \file resource-query.c
 *
 * Tests querying resources.
 *
 * From the GL_ARB_program_interface_query spec:
 *      "The command
 *
 *      void GetProgramInterfaceiv(uint program, enum programInterface,
 *                                 enum pname, int *params);
 *
 *      queries a property of the interface <programInterface> in program
 *      <program>, returning its value in <params>.  The property to return is
 *      specified by <pname>.
 *
 *      If <pname> is ACTIVE_RESOURCES, the value returned is the number of
 *      resources in the active resource list for <programInterface>. If the
 *      list of active resources for <programInterface> is empty, zero is
 *      returned.
 *
 *      If <pname> is MAX_NAME_LENGTH, the value returned is the length of the
 *      longest active name string for an active resource in <programInterface>.
 *      This length includes an extra character for the null terminator. If
 *      the list of active resources for <programInterface> is empty, zero is
 *      returned.  The error INVALID_OPERATION is generated if
 *      <programInterface> is ATOMIC_COUNTER_BUFFER, since active atomic counter
 *      buffer resources are not assigned name strings.
 *
 *      If <pname> is MAX_NUM_ACTIVE_VARIABLES, the value returned is the number
 *      of active variables belonging to the interface block or atomic counter
 *      buffer resource in <programInterface> with the most active variables.
 *      If the list of active resources for <programInterface> is empty, zero is
 *      returned.  The error INVALID_OPERATION is generated if
 *      <programInterface> is not UNIFORM_BLOCK, ATOMIC_COUNTER_BUFFER, or
 *      SHADER_STORAGE_BLOCK.
 *
 *      If <pname> is MAX_NUM_COMPATIBLE_SUBROUTINES, the value returned is the
 *      number of compatible subroutines belonging to the active subroutine
 *      uniform in <programInterface> with the most compatible subroutines. If
 *      the list of active resources for <programInterface> is empty, zero is
 *      returned.  The error INVALID_OPERATION is generated unless
 *      <programInterface> is VERTEX_SUBROUTINE_UNIFORM,
 *      TESS_CONTROL_SUBROUTINE_UNIFORM, TESS_EVALUATION_SUBROUTINE_UNIFORM,
 *      GEOMETRY_SUBROUTINE_UNIFORM, FRAGMENT_SUBROUTINE_UNIFORM, or
 *      COMPUTE_SUBROUTINE_UNIFORM.
 *
 *      The command
 *
 *      uint GetProgramResourceIndex(uint program, enum programInterface,
 *                                   const char *name);
 *
 *      returns the unsigned integer index assigned to a resource named <name>
 *      in the interface type <programInterface> of program object <program>.
 *      The error INVALID_ENUM is generated if <programInterface> is
 *      ATOMIC_COUNTER_BUFFER, since active atomic counter buffer resources are
 *      not assigned name strings.
 *
 *      If <name> exactly matches the name string of one of the active resources
 *      for <programInterface>, the index of the matched resource is returned.
 *      Additionally, if <name> would exactly match the name string of an active
 *      resource if "[0]" were appended to <name>, the index of the matched
 *      resource is returned.  Otherwise, <name> is considered not to be the
 *      name of an active resource, and INVALID_INDEX is returned.  Note that if
 *      an interface enumerates a single active resource list entry for an array
 *      variable (e.g., "a[0]"), a <name> identifying any array element other
 *      than the first (e.g., "a[1]") is not considered to match.
 *
 *      For the interface TRANSFORM_FEEDBACK_VARYING, the value INVALID_INDEX
 *      should be returned when querying the index assigned to the special names
 *      "gl_NextBuffer", "gl_SkipComponents1", "gl_SkipComponents2",
 *      "gl_SkipComponents3", and "gl_SkipComponents4".
 *
 *      The command
 *
 *      void GetProgramResourceName(uint program, enum programInterface,
 *                                  uint index, sizei bufSize, sizei *length,
 *                                  char *name);
 *
 *      returns the name string assigned to the single active resource with an
 *      index of <index> in the interface <programInterface> of program object
 *      <program>.  The error INVALID_VALUE is generated if <index> is greater
 *      than or equal to the number of entries in the active resource list for
 *      <programInterface>.  The error INVALID_ENUM is generated if
 *      <programInterface> is ATOMIC_COUNTER_BUFFER, since active atomic counter
 *      buffer resources are not assigned name strings.
 *
 *      The name string assigned to the active resource identified by <index> is
 *      returned as a null-terminated string in <name>.  The actual number of
 *      characters written into <name>, excluding the null terminator, is
 *      returned in <length>.  If <length> is NULL, no length is returned. The
 *      maximum number of characters that may be written into <name>, including
 *      the null terminator, is specified by <bufSize>.  If the length of the
 *      name string (including the null terminator) is greater than <bufSize>,
 *      the first <bufSize>-1 characters of the name string will be written to
 *      <name>, followed by a null terminator.  If <bufSize> is zero, no error
 *      will be generated but no characters will be written to <name>.  The
 *      length of the longest name string for <programInterface>, including a
 *      null terminator, can be queried by calling GetProgramInterfaceiv with a
 *      <pname> of MAX_NAME_LENGTH.
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

/* Naming conventions, from the GL_ARB_program_interface_query extension:
 *
 * "When building a list of active variable or interface blocks, resources
 * with aggregate types (such as arrays or structures) may produce multiple
 * entries in the active resource list for the corresponding interface.
 * Additionally, each active variable, interface block, or subroutine in the
 * list is assigned an associated name string that can be used by
 * applications to refer to the resources.  For interfaces involving
 * variables, interface blocks, or subroutines, the entries of active
 * resource lists are generated as follows:
 *
 *  * For an active variable declared as a single instance of a basic type,
 *    a single entry will be generated, using the variable name from the
 *    shader source.
 *
 *  * For an active variable declared as an array of basic types, a single
 *    entry will be generated, with its name string formed by concatenating
 *    the name of the array and the string "[0]".
 *
 *  * For an active variable declared as a structure, a separate entry will
 *    be generated for each active structure member.  The name of each entry
 *    is formed by concatenating the name of the structure, the "."
 *    character, and the name of the structure member.  If a structure
 *    member to enumerate is itself a structure or array, these enumeration
 *    rules are applied recursively.
 *
 *  * For an active variable declared as an array of an aggregate data type
 *    (structures or arrays), a separate entry will be generated for each
 *    active array element, unless noted immediately below.  The name of
 *    each entry is formed by concatenating the name of the array, the "["
 *    character, an integer identifying the element number, and the "]"
 *    character.  These enumeration rules are applied recursively, treating
 *    each enumerated array element as a separate active variable.
 *
 *  * For an active shader storage block member declared as an array, an
 *    entry will be generated only for the first array element, regardless
 *    of its type.  For arrays of aggregate types, the enumeration rules are
 *    applied recursively for the single enumerated array element.
 *
 *  * For an active interface block not declared as an array of block
 *    instances, a single entry will be generated, using the block name from
 *    the shader source.
 *
 *  * For an active interface block declared as an array of instances,
 *    separate entries will be generated for each active instance.  The name
 *    of the instance is formed by concatenating the block name, the "["
 *    character, an integer identifying the instance number, and the "]"
 *    character.
 *
 *  * For an active subroutine, a single entry will be generated, using the
 *    subroutine name from the shader source.
 *
 * When an integer array element or block instance number is part of the name
 * string, it will be specified in decimal form without a "+" or "-" sign or
 * any extra leading zeroes.  Additionally, the name string will not include
 * white space anywhere in the string.
 */
static const char *st_r_uniform[] = {"vs_test", "gs_test", "fs_color",
				     "fs_array[0]", "sa[0].a[0]", "sa[1].a[0]",
				     NULL};
static const char *st_r_tess_uniform[] = {"tcs_test", "tes_test", NULL};
static const char *st_r_cs_uniform[] = {"cs_test", "tex", NULL};
static const char *st_r_uniform_block[] = {"vs_uniform_block",
					   "gs_uniform_block",
					   "fs_uniform_block", NULL};
static const char *st_r_tess_uniform_block[] = {"tcs_uniform_block",
						"tes_uniform_block", NULL};
static const char *st_r_cs_uniform_block[] = {"cs_uniform_block", NULL};
static const char *st_r_in_vs[] = {"vs_input0", "vs_input1", NULL};
static const char *st_r_in_gs[] = {"gs_input", "gl_Position", NULL};
static const char *st_r_in_fs[] = {"fs_input1", NULL};
static const char *st_r_in_tes[] = {"tes_input", "gl_Position", NULL};
static const char *st_r_in_tcs[] = {"tcs_input", "gl_Position", NULL};
static const char *st_r_out_vs[] = {"gl_Position", NULL};
static const char *st_r_out_gs[] = {"gs_output0", "gl_Position", NULL};
static const char *st_r_out_fs[] = {"fs_output0", "fs_output1", NULL};
static const char *st_r_out_tes[] = {"tes_output[0]", "gl_Position", NULL};
static const char *st_r_out_tcs[] = {"tcs_output", "tcs_patch", "gl_Position",
				     "gl_BackColor", "gl_BackSecondaryColor",
				     "gl_ClipDistance[0]", "gl_CullDistance[0]",
				     "gl_FogFragCoord", "gl_FrontColor",
				     "gl_FrontSecondaryColor", "gl_Layer",
				     "gl_PointSize", "gl_TexCoord[0]",
				     "gl_ViewportIndex", "gl_ViewportMask[0]",
				     NULL};
static const char *st_r_buffer[] = {"vs_buf_var", "gs_buf_var", "fs_buf_var",
				    NULL};
static const char *st_r_stor_block[] = {"vs_buffer_block", "gs_buffer_block",
					"fs_buffer_block", NULL};
static const char *st_r_tf_varying[] = {"gl_Position", "gs_output0", NULL};
static const char *st_r_vs_sub[] = {"vss", "vss2", NULL};
static const char *st_r_gs_sub[] = {"gss", NULL};
static const char *st_r_fs_sub[] = {"fss", NULL};
static const char *st_r_cs_sub[] = {"css", NULL};
static const char *st_r_tcs_sub[] = {"tcss", NULL};
static const char *st_r_tes_sub[] = {"tess", NULL};
static const char *st_r_vs_sub_uni[] = {"VERTEX", NULL};
static const char *st_r_gs_sub_uni[] = {"GEOMETRY", NULL};
static const char *st_r_fs_sub_uni[] = {"FRAGMENT", NULL};
static const char *st_r_cs_sub_uni[] = {"COMPUTE", NULL};
static const char *st_r_tcs_sub_uni[] = {"TESS_CONTROL", NULL};
static const char *st_r_tes_sub_uni[] = {"TESS_EVALUATION", NULL};

/* From the GL_ARB_program_interface_query extension:
 *
 * "The GL provides a number of commands to query properties of the interfaces
 * of a program object.  Each such command accepts a <programInterface>
 * token, identifying a specific interface.  The supported values for
 * <programInterface> are as follows:
 *  * UNIFORM corresponds to the set of active uniform variables (section
 *    2.14.7) used by <program>.
 *
 *  * UNIFORM_BLOCK corresponds to the set of active uniform blocks (section
 *    2.14.7) used by <program>.
 *
 *  * ATOMIC_COUNTER_BUFFER corresponds to the set of active atomic counter
 *    buffer binding points (section 2.14.7) used by <program>.

 *  * PROGRAM_INPUT corresponds to the set of active input variables used by
 *    the first shader stage of <program>.  If <program> includes multiple
 *    shader stages, input variables from any shader stage other than the
 *    first will not be enumerated.
 *
 *  * PROGRAM_OUTPUT corresponds to the set of active output variables
 *    (section 2.14.11) used by the last shader stage of <program>.  If
 *    <program> includes multiple shader stages, output variables from any
 *    shader stage other than the last will not be enumerated.
 *
 *  * VERTEX_SUBROUTINE, TESS_CONTROL_SUBROUTINE,
 *    TESS_EVALUATION_SUBROUTINE, GEOMETRY_SUBROUTINE, FRAGMENT_SUBROUTINE,
 *    and COMPUTE_SUBROUTINE correspond to the set of active subroutines for
 *    the vertex, tessellation control, tessellation evaluation, geometry,
 *    fragment, and compute shader stages of <program>, respectively
 *    (section 2.14.8).
 *
 *  * VERTEX_SUBROUTINE_UNIFORM, TESS_CONTROL_SUBROUTINE_UNIFORM,
 *    TESS_EVALUATION_SUBROUTINE_UNIFORM, GEOMETRY_SUBROUTINE_UNIFORM,
 *    FRAGMENT_SUBROUTINE_UNIFORM, and COMPUTE_SUBROUTINE_UNIFORM correspond
 *    to the set of active subroutine uniform variables used by the vertex,
 *    tessellation control, tessellation evaluation, geometry, fragment, and
 *    compute shader stages of <program>, respectively (section 2.14.8).
 *
 *  * TRANSFORM_FEEDBACK_VARYING corresponds to the set of output variables
 *    in the last non-fragment stage of <program> that would be captured
 *    when transform feedback is active (section 2.20.2).
 *
 *  * BUFFER_VARIABLE corresponds to the set of active buffer variables (see
 *    the ARB_shader_storage_buffer_object extension) used by <program>.
 *
 *  * SHADER_STORAGE_BLOCK corresponds to the set of active shader storage
 *    blocks (see the ARB_shader_storage_buffer_object extension) used by
 *    <program>."
 *
 * Additionally, from the GL_ARB_program_interface_query extension:
 *
 * "For the ATOMIC_COUNTER_BUFFER interface, the list of active buffer binding
 * points is built by identifying each unique binding point associated with
 * one or more active atomic counter uniform variables.  Active atomic
 * counter buffers do not have an associated name string.
 *
 * For the UNIFORM, PROGRAM_INPUT, PROGRAM_OUTPUT, and
 * TRANSFORM_FEEDBACK_VARYING interfaces, the active resource list will
 * include all active variables for the interface, including any active
 * built-in variables.
 *
 * For PROGRAM_INPUT and PROGRAM_OUTPUT interfaces for shaders that recieve
 * or produce patch primitves, the active resource list will include both
 * per-vertex and per-patch inputs and outputs.
 *
 * For the TRANSFORM_FEEDBACK_VARYING interface, the active resource list
 * will entries for the special varying names gl_NextBuffer,
 * gl_SkipComponents1, gl_SkipComponents2, gl_SkipComponents3, and
 * gl_SkipComponents4 (section 2.14.11).  These variables are used to control
 * how varying values are written to transform feedback buffers.  When
 * enumerating the properties of such resources, these variables are
 * considered to have a TYPE of NONE and an ARRAY_SIZE of 0 (gl_NextBuffer),
 * 1, 2, 3, and 4, respectively."
 */

struct subtest_t {
	GLenum programInterface;

	const char *programInterface_str;
	const char *active_resources_str;
	const char *max_length_name_str;
	const char *max_num_active_str;
	const char *max_num_compat_sub_str;

	/* set to -1 to disable the test */
	int active_resources;
	int max_length_name;
	int max_num_active;
	int max_num_compat_sub;

	const char *vs_text;
	const char *gs_text;
	const char *fs_text;
	const char *tcs_text;
	const char *tes_text;
	const char *cs_text;

	const char **resources;
};

#define ST(active_r, max_len, max_num_active, max_num_compat_sub, vs, tcs, \
	   tes, gs, fs, cs, name, suffix, resources) { \
	(name), #name suffix, #name suffix " active resources", \
	#name suffix " max length name", \
	#name suffix " max num active", \
	#name suffix " max num compat sub", \
	(active_r), (max_len), (max_num_active), (max_num_compat_sub), \
	(vs), (gs), (fs), (tcs), (tes), (cs), (resources) \
}

static const struct subtest_t subtests[] = {
 ST( 6, 12, -1, -1,  vs_std,    NULL,    NULL,  gs_std,  fs_std,   NULL, GL_UNIFORM, "(vs,gs,fs)", st_r_uniform),
 ST( 2,  9, -1, -1,    NULL, tcs_sub, tes_sub,    NULL,    NULL,   NULL, GL_UNIFORM, "(tes,tcs)", st_r_tess_uniform),
 ST( 2,  8, -1, -1,    NULL,    NULL,    NULL,    NULL,    NULL, cs_sub, GL_UNIFORM, "(cs)", st_r_cs_uniform),
 ST( 3, 17,  2, -1,  vs_std,    NULL,    NULL,  gs_std,  fs_std,   NULL, GL_UNIFORM_BLOCK, "(vs,gs,fs)", st_r_uniform_block),
 ST( 2, 18, -1, -1,    NULL, tcs_sub, tes_sub,    NULL,    NULL,   NULL, GL_UNIFORM_BLOCK, "(tcs,tes)", st_r_tess_uniform_block),
 ST( 1, 17, -1, -1,    NULL,    NULL,    NULL,    NULL,    NULL, cs_sub, GL_UNIFORM_BLOCK, "(cs)", st_r_cs_uniform_block),
 ST( 2, 10, -1, -1,  vs_std,    NULL,    NULL,    NULL,    NULL,   NULL, GL_PROGRAM_INPUT, "(vs)", st_r_in_vs),
 ST( 2, 12, -1, -1,    NULL,    NULL,    NULL,  gs_std,    NULL,   NULL, GL_PROGRAM_INPUT, "(gs)", st_r_in_gs),
 ST( 1, 10, -1, -1,    NULL,    NULL,    NULL,    NULL,  fs_std,   NULL, GL_PROGRAM_INPUT, "(fs)", st_r_in_fs),
 ST( 2, 10, -1, -1,  vs_std,    NULL,    NULL,    NULL,  fs_std,   NULL, GL_PROGRAM_INPUT, "(vs,fs)", st_r_in_vs),
 ST( 2, 10, -1, -1,  vs_std,    NULL,    NULL,  gs_std,    NULL,   NULL, GL_PROGRAM_INPUT, "(vs,gs)", st_r_in_vs),
 ST( 2, 12, -1, -1,    NULL,    NULL,    NULL,  gs_std,  fs_std,   NULL, GL_PROGRAM_INPUT, "(gs,fs)", st_r_in_gs),
 ST( 2, 10, -1, -1,  vs_std,    NULL,    NULL,  gs_std,  fs_std,   NULL, GL_PROGRAM_INPUT, "(vs,gs,fs)", st_r_in_vs),
 ST( 2, 12, -1, -1,    NULL,    NULL, tes_sub,    NULL,    NULL,   NULL, GL_PROGRAM_INPUT, "(tes)", st_r_in_tes),
 ST( 2, 12, -1, -1,    NULL, tcs_sub,    NULL,    NULL,    NULL,   NULL, GL_PROGRAM_INPUT, "(tcs)", st_r_in_tcs),
 ST( 2, 12, -1, -1,    NULL, tcs_sub, tes_sub,    NULL,    NULL,   NULL, GL_PROGRAM_INPUT, "(tcs,tes)", st_r_in_tcs),
 ST( 2, 10, -1, -1,  vs_std, tcs_sub, tes_sub,    NULL,    NULL,   NULL, GL_PROGRAM_INPUT, "(vs,tcs,tes)", st_r_in_vs),
 ST( 0,  0, -1, -1,    NULL,    NULL,    NULL,    NULL,    NULL, cs_sub, GL_PROGRAM_INPUT, "(cs)", NULL),
 ST( 1, 12, -1, -1,  vs_std,    NULL,    NULL,    NULL,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(vs)", st_r_out_vs),
 ST( 2, 12, -1, -1,    NULL,    NULL,    NULL,  gs_std,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(gs)", st_r_out_gs),
 ST( 2, 11, -1, -1,    NULL,    NULL,    NULL,    NULL,  fs_std,   NULL, GL_PROGRAM_OUTPUT, "(fs)", st_r_out_fs),
 ST( 2, 11, -1, -1,  vs_std,    NULL,    NULL,    NULL,  fs_std,   NULL, GL_PROGRAM_OUTPUT, "(vs,fs)", st_r_out_fs),
 ST( 2, 12, -1, -1,  vs_std,    NULL,    NULL,  gs_std,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(vs,gs)", st_r_out_gs),
 ST( 2, 11, -1, -1,    NULL,    NULL,    NULL,  gs_std,  fs_std,   NULL, GL_PROGRAM_OUTPUT, "(gs,fs)", st_r_out_fs),
 ST( 2, 11, -1, -1,  vs_std,    NULL,    NULL,  gs_std,  fs_std,   NULL, GL_PROGRAM_OUTPUT, "(vs,gs,fs)", st_r_out_fs),
 ST( 2, 14, -1, -1,    NULL,    NULL, tes_sub,    NULL,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(tes)", st_r_out_tes),
 ST(15, 23, -1, -1,    NULL, tcs_sub,    NULL,    NULL,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(tcs)", st_r_out_tcs),
 ST( 2, 14, -1, -1,    NULL, tcs_sub, tes_sub,    NULL,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(tcs,tes)", st_r_out_tes),
 ST( 2, 12, -1, -1,    NULL, tcs_sub, tes_sub,  gs_std,    NULL,   NULL, GL_PROGRAM_OUTPUT, "(tcs,tes,gs)", st_r_out_gs),
 ST( 0,  0, -1, -1,    NULL,    NULL,    NULL,    NULL,    NULL, cs_sub, GL_PROGRAM_OUTPUT, "(cs)", st_r_cs_sub),
 ST( 3, 11, -1, -1, vs_stor,    NULL,    NULL, gs_stor, fs_stor,   NULL, GL_BUFFER_VARIABLE, "", st_r_buffer),
 ST( 3, 16,  1, -1, vs_stor,    NULL,    NULL, gs_stor, fs_stor,   NULL, GL_SHADER_STORAGE_BLOCK, "", st_r_stor_block),
 ST( 3, -1,  1, -1, vs_atom,    NULL,    NULL, gs_atom, fs_atom,   NULL, GL_ATOMIC_COUNTER_BUFFER, "", NULL),
 ST( 2, 12, -1, -1,  vs_std,    NULL,    NULL,  gs_std,    NULL,   NULL, GL_TRANSFORM_FEEDBACK_VARYING, "", st_r_tf_varying),
 ST( 2,  5, -1, -1,  vs_sub,    NULL,    NULL,    NULL,    NULL,   NULL, GL_VERTEX_SUBROUTINE, "", st_r_vs_sub),
 ST( 1,  4, -1, -1,  vs_sub,    NULL,    NULL,  gs_sub,    NULL,   NULL, GL_GEOMETRY_SUBROUTINE, "", st_r_gs_sub),
 ST( 1,  4, -1, -1,  vs_sub,    NULL,    NULL,  gs_sub,  fs_sub,   NULL, GL_FRAGMENT_SUBROUTINE, "", st_r_fs_sub),
 ST( 1,  4, -1, -1,    NULL,    NULL,    NULL,    NULL,    NULL, cs_sub, GL_COMPUTE_SUBROUTINE, "", st_r_cs_sub),
 ST( 1,  5, -1, -1,  vs_sub,    tcs_sub, NULL,    NULL,    NULL,   NULL, GL_TESS_CONTROL_SUBROUTINE, "", st_r_tcs_sub),
 ST( 1,  5, -1, -1,  vs_sub,    NULL, tes_sub,    NULL,    NULL,   NULL, GL_TESS_EVALUATION_SUBROUTINE, "", st_r_tes_sub),
 ST( 1,  7, -1,  2,  vs_sub,    NULL,    NULL,    NULL,    NULL,   NULL, GL_VERTEX_SUBROUTINE_UNIFORM, "", st_r_vs_sub_uni),
 ST( 1,  9, -1,  1,  vs_sub,    NULL,    NULL,  gs_sub,    NULL,   NULL, GL_GEOMETRY_SUBROUTINE_UNIFORM, "", st_r_gs_sub_uni),
 ST( 1,  9, -1,  1,  vs_sub,    NULL,    NULL,  gs_sub,  fs_sub,   NULL, GL_FRAGMENT_SUBROUTINE_UNIFORM, "", st_r_fs_sub_uni),
 ST( 1, 13, -1,  1,  vs_sub,    tcs_sub, NULL,    NULL,    NULL,   NULL, GL_TESS_CONTROL_SUBROUTINE_UNIFORM, "", st_r_tcs_sub_uni),
 ST( 1, 16, -1,  1,  vs_sub,    NULL, tes_sub,    NULL,    NULL,   NULL, GL_TESS_EVALUATION_SUBROUTINE_UNIFORM, "", st_r_tes_sub_uni),
 ST( 1,  8, -1,  1,    NULL,    NULL,    NULL,    NULL,    NULL, cs_sub, GL_COMPUTE_SUBROUTINE_UNIFORM, "", st_r_cs_sub_uni),
};

static void
check_pname(GLuint prog, GLenum programInterface, GLenum pname, bool *pass,
	    const char *subtest, int expected_value)
{
	int value;

	if (expected_value < 0) {
		return;
	}

	glGetProgramInterfaceiv(prog, programInterface, pname, &value);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("	Latest error generated while running '%s'\n",
		       subtest);
	}

	if (value != expected_value) {
		fprintf(stderr, "'%s' expected %i but got %i\n", subtest,
			expected_value, value);
		*pass = false;
	}
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

static bool
consistency_check(GLuint prog, GLenum programInterface, const char *name,
		  GLint index)
{
	bool subroutine = false;
	const GLchar *names[] = { name };
	GLuint old_idx = 0xdeadcafe;
	GLenum shader;

	/* Validate result against old API. */
	switch (programInterface) {
	case GL_UNIFORM:
		glGetUniformIndices(prog, 1, names, &old_idx);
		piglit_check_gl_error(GL_NO_ERROR);
		break;

	case GL_UNIFORM_BLOCK:
		old_idx = glGetUniformBlockIndex(prog, name);
		piglit_check_gl_error(GL_NO_ERROR);
		break;

	case GL_VERTEX_SUBROUTINE:
		shader = GL_VERTEX_SHADER;
		subroutine = true;
		break;

	case GL_TESS_CONTROL_SUBROUTINE:
		shader = GL_TESS_CONTROL_SHADER;
		subroutine = true;
		break;

	case GL_TESS_EVALUATION_SUBROUTINE:
		shader = GL_TESS_EVALUATION_SHADER;
		subroutine = true;
		break;

	case GL_GEOMETRY_SUBROUTINE:
		shader = GL_GEOMETRY_SHADER;
		subroutine = true;
		break;

	case GL_FRAGMENT_SUBROUTINE:
		shader = GL_FRAGMENT_SHADER;
		subroutine = true;
		break;

	case GL_COMPUTE_SUBROUTINE:
		shader = GL_COMPUTE_SHADER;
		subroutine = true;
		break;

	default:
		/* There are no old APIs for this program interface */
		return true;
	}

	if (subroutine) {
		old_idx = glGetSubroutineIndex(prog, shader, name);
		piglit_check_gl_error(GL_NO_ERROR);
	}

	if (index != old_idx) {
		printf("Index inconsistent with the old API: %i vs %i\n", index,
		       old_idx);
		return false;
	} else
		return true;
}


static void
validate_resources(const struct subtest_t st, GLuint prog, bool *pass)
{
	GLsizei max_size = 0, size, i;
	char * name;

	/* Do not run the test for GL_ATOMIC_COUNTER_BUFFER.
	 * From the GL_ARB_program_interface_query extension:
	 *
	 * "The error INVALID_OPERATION is generated if <programInterface>
	 * is ATOMIC_COUNTER_BUFFER, since active atomic counter buffer
	 * resources are not assigned name strings."
	 */
	if (st.programInterface == GL_ATOMIC_COUNTER_BUFFER)
		return;

	name = (char *) malloc(st.max_length_name);
	for (i = 0; i < st.active_resources; i++) {
		GLuint index;

		glGetProgramResourceName(prog, st.programInterface,
					 i, st.max_length_name,
					 &size, name);
		piglit_check_gl_error(GL_NO_ERROR);

		/* keep track of the maximum size */
		if (size > max_size) {
			max_size = size;
		}

		/* Check the names. Transform feedback requires the order to be
		 * the same as the one given in glTransformFeedbackVaryings.
		 * From the GL_ARB_program_interface_query extension:
		 *
		 * "The order of the active resource list is
		 * implementation-dependent for all interfaces except for
		 * TRANSFORM_FEEDBACK_VARYING. For TRANSFORM_FEEDBACK_VARYING,
		 * the active resource list will use the variable order
		 * specified in the the most recent call to
		 * TransformFeedbackVaryings before the last call to
		 * LinkProgram.
		 */
		if (st.resources && !is_resource_in_list(st.resources, name, i,
			st.programInterface == GL_TRANSFORM_FEEDBACK_VARYING)) {
			fprintf(stderr, "Resource '%s' not found in '%s' "
					"resource list or found at the wrong "
					"index\n", name,
				st.programInterface_str);
			*pass = false;
		}

		/* Check the position of the arguments and see if it matches
		 * with the current position we are in.
		 */
		index = glGetProgramResourceIndex(prog, st.programInterface,
						  name);
		if (index != i) {
			fprintf(stderr, "%s: Resource '%s' is not at the "
					"position reported by "
					"glGetProgramResourceIndex (%i instead "
					"of %i)\n",
				st.programInterface_str, name, index, i);
			*pass = false;
		}

		/* check the equivalence with the old API */
		if (!consistency_check(prog, st.programInterface, name,
				       index)) {
			*pass = false;
		}
	}
	free(name);

	/* glGetProgramResourceName does not count the NULL terminator as part
	 * of the size contrarily to glGetProgramInterfaceiv.
	 * From the GL_ARB_program_interface_query extension:
	 *
	 * "void GetProgramInterfaceiv(uint program, enum programInterface,
	 *                             enum pname, int *params);
	 * [...]
	 * If <pname> is MAX_NAME_LENGTH, the value returned is the length of
	 * the longest active name string for an active resource in
	 * <programInterface>. This length includes an extra character for the
	 * null terminator."
	 *
	 * "void GetProgramResourceName(uint program, enum programInterface,
	 *                              uint index, sizei bufSize,
	 *                              sizei *length, char *name);
	 * [...]
	 * The actual number of characters written into <name>, excluding the
	 * null terminator, is returned in <length>."
	 */
	if (max_size != MAX2(0, st.max_length_name - 1)) {
		fprintf(stderr, "'%s actual max length' expected %i but got "
				"%i\n", st.programInterface_str,
			st.max_length_name - 1,	max_size);
		*pass = false;
	}
}

static bool
check_extensions(const struct subtest_t st)
{
	if (st.programInterface == GL_ATOMIC_COUNTER_BUFFER &&
	    !piglit_is_extension_supported("GL_ARB_shader_atomic_counters")) {
		return false;
	}

	if ((st.programInterface == GL_BUFFER_VARIABLE ||
	    st.programInterface == GL_SHADER_STORAGE_BLOCK) &&
	    !piglit_is_extension_supported("GL_ARB_shader_storage_buffer_object")) {
		return false;
	}

	if ((st.programInterface == GL_VERTEX_SUBROUTINE ||
	     st.programInterface == GL_GEOMETRY_SUBROUTINE ||
	     st.programInterface == GL_FRAGMENT_SUBROUTINE ||
	     st.programInterface == GL_COMPUTE_SUBROUTINE ||
	     st.programInterface == GL_VERTEX_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_GEOMETRY_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_FRAGMENT_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_COMPUTE_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_TESS_CONTROL_SUBROUTINE ||
	     st.programInterface == GL_TESS_EVALUATION_SUBROUTINE ||
	     st.programInterface == GL_TESS_CONTROL_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_TESS_EVALUATION_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_COMPUTE_SUBROUTINE_UNIFORM) &&
	     !piglit_is_extension_supported("GL_ARB_shader_subroutine")) {
		 return false;
	 }

	if ((st.programInterface == GL_TESS_CONTROL_SUBROUTINE ||
	     st.programInterface == GL_TESS_EVALUATION_SUBROUTINE ||
	     st.programInterface == GL_TESS_CONTROL_SUBROUTINE_UNIFORM ||
	     st.programInterface == GL_TESS_EVALUATION_SUBROUTINE_UNIFORM ||
	     st.tcs_text || st.tes_text) &&
	     !piglit_is_extension_supported("GL_ARB_tessellation_shader")) {
		 return false;
	 }

	if ((st.programInterface == GL_COMPUTE_SUBROUTINE ||
	     st.programInterface == GL_COMPUTE_SUBROUTINE_UNIFORM ||
	     st.cs_text) &&
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
	GLuint prog;

	if (!check_extensions(st)) {
		result = PIGLIT_SKIP;
		goto report_result;
	}

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
					GL_VERTEX_SHADER,   st.vs_text,
					GL_GEOMETRY_SHADER, st.gs_text,
					GL_FRAGMENT_SHADER, st.fs_text,
					GL_TESS_CONTROL_SHADER, st.tcs_text,
					GL_TESS_EVALUATION_SHADER, st.tes_text,
					GL_COMPUTE_SHADER, st.cs_text,
					0);

	if (st.programInterface == GL_TRANSFORM_FEEDBACK_VARYING) {
		glTransformFeedbackVaryings(prog, 2, st_r_tf_varying,
					    GL_INTERLEAVED_ATTRIBS);
		piglit_check_gl_error(GL_NO_ERROR);
	}

	/* force the compiler not to optimise away inputs/outputs */
	glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, GL_TRUE);
	piglit_check_gl_error(GL_NO_ERROR);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		result = PIGLIT_FAIL;
		*pass = false;
		goto report_result;
	}

	check_pname(prog, st.programInterface, GL_ACTIVE_RESOURCES,
		    &local_pass, st.active_resources_str, st.active_resources);

	check_pname(prog, st.programInterface, GL_MAX_NAME_LENGTH,
		    &local_pass, st.max_length_name_str, st.max_length_name);

	/* do not test fetching the names if the previous tests failed */
	if (local_pass) {
		validate_resources(st, prog, &local_pass);
	}

	check_pname(prog, st.programInterface, GL_MAX_NUM_ACTIVE_VARIABLES,
		    &local_pass, st.max_num_active_str, st.max_num_active);

	check_pname(prog, st.programInterface,
		    GL_MAX_NUM_COMPATIBLE_SUBROUTINES, &local_pass,
		    st.max_num_compat_sub_str, st.max_num_compat_sub);

	glDeleteProgram(prog);

	*pass = *pass && local_pass;
	result = local_pass ? PIGLIT_PASS : PIGLIT_FAIL;

report_result:
	piglit_report_subtest_result(result, "%s", st.programInterface_str);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_program_interface_query");
	piglit_require_extension("GL_ARB_separate_shader_objects");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i;

	/* run all the getprograminterfaceiv tests */
	for (i = 0; i < sizeof(subtests) / sizeof(struct subtest_t); i++) {
		run_subtest(subtests[i], &pass);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
