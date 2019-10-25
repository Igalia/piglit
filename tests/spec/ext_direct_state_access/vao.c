/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

/* This test verifies that the VAO functions added by EXT_direct_state_access
 * modifies the specified VAO object instead of the bound one.
 * Validation of the correct interpretation of the parameters is left to the
 * other VAO specific tests.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint vao[2];
static GLuint buffers[11];

static bool
verify(const char* func, GLenum pname, GLintptr expected_value, bool is_pointer, int attrib_index)
{
	GLintptr v;
	char* get_fn;
	if (is_pointer) {
		void* ptr;
		if (attrib_index >= 0) {
			glGetVertexAttribPointerv(attrib_index, pname, &ptr);
			get_fn = "glGetVertexAttribPointerv";
		} else {
			glGetPointerv(pname, &ptr);
			get_fn = "glGetPointerv";
		}
		v = (GLintptr) ptr;
	} else {
		int d;
		if (attrib_index >= 0) {
			glGetVertexAttribiv(attrib_index, pname, &d);
			get_fn = "glGetVertexAttribiv";
		} else {
			glGetIntegerv(pname, &d);
			get_fn = "glGetIntegerv";
		}
		v = d;
	}

	if (v == expected_value)
		return true;

	if (is_pointer)
		printf("gl%s error: expected value is %p but %s(%s) read %p\n",
			func + 5, (void*) expected_value, get_fn, piglit_get_gl_enum_name(pname), (void*) v);
	else
		printf("gl%s error: expected value is %d but %s(%s) read %d\n",
			func + 5, (int) expected_value, get_fn, piglit_get_gl_enum_name(pname), (int) v);
	return false;
}

static enum piglit_result
test_VertexArrayVertexOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayVertexOffsetEXT(vao[1], buffer, 3, GL_DOUBLE, 24, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_VERTEX_ARRAY_SIZE, 3, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ARRAY_TYPE, GL_DOUBLE, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ARRAY_STRIDE, 24, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayColorOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayColorOffsetEXT(vao[1], buffer, 3, GL_FLOAT, 13, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_COLOR_ARRAY_SIZE, 3, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_COLOR_ARRAY_TYPE, GL_FLOAT, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_COLOR_ARRAY_STRIDE, 13, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_COLOR_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_COLOR_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayEdgeFlagOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayEdgeFlagOffsetEXT(vao[1], buffer, 56, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_EDGE_FLAG_ARRAY_STRIDE, 56, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_EDGE_FLAG_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_EDGE_FLAG_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayIndexOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayIndexOffsetEXT(vao[1], buffer, GL_UNSIGNED_BYTE, 12, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_INDEX_ARRAY_TYPE, GL_UNSIGNED_BYTE, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_INDEX_ARRAY_STRIDE, 12, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_INDEX_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_INDEX_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayNormalOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayNormalOffsetEXT(vao[1], buffer, GL_DOUBLE, 4, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_NORMAL_ARRAY_TYPE, GL_DOUBLE, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_NORMAL_ARRAY_STRIDE, 4, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_NORMAL_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_NORMAL_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayTexCoordOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayTexCoordOffsetEXT(vao[1], buffer, 2, GL_INT, 24, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_SIZE, 2, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_TYPE, GL_INT, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_STRIDE, 24, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayMultiTexCoordOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayMultiTexCoordOffsetEXT(vao[1], buffer, GL_TEXTURE3,
						    3, GL_SHORT, 10, offset);

		glBindVertexArray(vao[1]);
		glClientActiveTexture(GL_TEXTURE3);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_SIZE, 3, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_TYPE, GL_SHORT, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_STRIDE, 10, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);
		glClientActiveTexture(GL_TEXTURE0);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayFogCoordOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArrayFogCoordOffsetEXT(vao[1], buffer, GL_DOUBLE, 36, offset);

		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_FOG_COORD_ARRAY_TYPE, GL_DOUBLE, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_FOG_COORD_ARRAY_STRIDE, 36, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_FOG_COORD_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_FOG_COORD_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArraySecondaryColorOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;

		glVertexArraySecondaryColorOffsetEXT(vao[1], buffer, 3, GL_DOUBLE, 12, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_SECONDARY_COLOR_ARRAY_SIZE, 3, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_SECONDARY_COLOR_ARRAY_TYPE, GL_DOUBLE, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_SECONDARY_COLOR_ARRAY_STRIDE, 12, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING, buffer, false, -1) && pass;
		pass = verify(__FUNCTION__, GL_SECONDARY_COLOR_ARRAY_POINTER, offset, true, -1) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayVertexAttribOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;
		GLuint index = 5;

		glVertexArrayVertexAttribOffsetEXT(vao[1], buffer, index, 3, GL_DOUBLE, true, 8, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_SIZE, 3, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_TYPE, GL_DOUBLE, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, 1, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_STRIDE, 8, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, buffer, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_POINTER, offset, true, index) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_VertexArrayVertexAttribIOffsetEXT(void* d)
{
	bool pass = true;
	for (int i = 0; i < 2; i++) {
		GLuint buffer = i ? buffers[rand() % ARRAY_SIZE(buffers)] : 0;
		GLintptr offset = buffer ? abs(rand()) : 0;
		GLuint index = 3;

		glVertexArrayVertexAttribIOffsetEXT(vao[1], buffer, index, 2, GL_UNSIGNED_INT, 10, offset);
		glBindVertexArray(vao[1]);

		/* Read back the values */
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_SIZE, 2, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_TYPE, GL_UNSIGNED_INT, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_STRIDE, 10, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, buffer, false, index) && pass;
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_POINTER, offset, true, index) && pass;

		glBindVertexArray(vao[0]);

		if (buffer)
			pass = glIsBuffer(buffer) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_EnableVertexArrayEXT(void* d)
{
	bool pass = true;
	GLenum caps[] = {
		GL_COLOR_ARRAY,
		GL_EDGE_FLAG_ARRAY,
		GL_FOG_COORD_ARRAY,
		GL_INDEX_ARRAY,
		GL_NORMAL_ARRAY,
		GL_SECONDARY_COLOR_ARRAY,
		GL_TEXTURE_COORD_ARRAY,
		GL_VERTEX_ARRAY,
	/* The EXT_direct_state_access spec says:
	 *
	 *    "Additionally EnableVertexArrayEXT and DisableVertexArrayEXT accept
	 *    the tokens TEXTURE0 through TEXTUREn where n is less than the
	 *    implementation-dependent limit of MAX_TEXTURE_COORDS.  For these
	 *    GL_TEXTUREi tokens, EnableVertexArrayEXT and DisableVertexArrayEXT
	 *    act identically to EnableVertexArrayEXT(vaobj, TEXTURE_COORD_ARRAY)
	 *    or DisableVertexArrayEXT(vaobj, TEXTURE_COORD_ARRAY) respectively
	 *    as if the active client texture is set to texture coordinate set i
	 *    based on the token TEXTUREi indicated by array."
	 */
		GL_TEXTURE3,
	};
	for (int i = 0; i < ARRAY_SIZE(caps); i++) {
		int enabled;

		glEnableVertexArrayEXT(vao[1], caps[i]);

		glBindVertexArray(vao[1]);
		if (caps[i] == GL_TEXTURE3) {
			glClientActiveTexture(GL_TEXTURE3);
			glGetIntegerv(GL_TEXTURE_COORD_ARRAY, &enabled);
		} else
			glGetIntegerv(caps[i], &enabled);
		pass = enabled && pass;
		glBindVertexArray(vao[0]);
		glClientActiveTexture(GL_TEXTURE0);

		glDisableVertexArrayEXT(vao[1], caps[i]);

		glBindVertexArray(vao[1]);
		if (caps[i] == GL_TEXTURE3) {
			glClientActiveTexture(GL_TEXTURE3);
			glGetIntegerv(GL_TEXTURE_COORD_ARRAY, &enabled);
		} else
			glGetIntegerv(caps[i], &enabled);
		pass = !enabled && pass;
		glBindVertexArray(vao[0]);
		glClientActiveTexture(GL_TEXTURE0);
	}

	/* The EXT_direct_state_access spec says:
	 *
	 *    "[EnableVertexArrayAttribEXT and DisableVertexArrayAttribEXT] operate
	 *    identically to [...] EnableVertexAttribArray, and DisableVertexAttribArray
	 *    respectively except rather than updating the current vertex array client-state
	 *    these "VertexArray" commands update the vertex array enables
	 *    within the vertex array object named by the initial vaobj parameter.
	 *    [...].  The index parameter matches the index parameter for the corresponding
	 *    EnableVertexAttribArray and DisableVertexAttribArray commands.
	 */
	int max_attribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attribs);
	for (int i = 0; i < max_attribs; i++) {
		int enabled;
		glEnableVertexArrayAttribEXT(vao[1], i);
		glBindVertexArray(vao[1]);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
		pass = enabled && pass;
		glBindVertexArray(vao[0]);

		glDisableVertexArrayAttribEXT(vao[1], i);
		glBindVertexArray(vao[1]);
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
		pass = !enabled && pass;
		glBindVertexArray(vao[0]);
	}

	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}


static enum piglit_result
test_GetVertexArrayIntegervEXT(void* d)
{
	bool pass = true;
	/* The EXT_direct_state_access spec says:
	 *
	 *    "For GetVertexArrayIntegervEXT, pname must be one of the "Get value" tokens
	 *    in tables 6.6, 6.7, 6.8, and 6.9 that use GetIntegerv, IsEnabled, or
	 *    GetPointerv for their "Get command" (so excluding the VERTEX_ATTRIB_*
	 *    tokens)."
	 */
	static const GLenum pnames_GetIntegerv[] = {
		GL_CLIENT_ACTIVE_TEXTURE,
		GL_VERTEX_ARRAY_SIZE,
		GL_VERTEX_ARRAY_TYPE,
		GL_VERTEX_ARRAY_STRIDE,
		GL_VERTEX_ARRAY_BUFFER_BINDING,
		GL_COLOR_ARRAY_SIZE,
		GL_COLOR_ARRAY_TYPE,
		GL_COLOR_ARRAY_STRIDE,
		GL_COLOR_ARRAY_BUFFER_BINDING,
		GL_EDGE_FLAG_ARRAY_STRIDE,
		GL_EDGE_FLAG_ARRAY_BUFFER_BINDING,
		GL_INDEX_ARRAY_TYPE,
		GL_INDEX_ARRAY_STRIDE,
		GL_INDEX_ARRAY_BUFFER_BINDING,
		GL_NORMAL_ARRAY_TYPE,
		GL_NORMAL_ARRAY_STRIDE,
		GL_NORMAL_ARRAY_BUFFER_BINDING,
		GL_TEXTURE_COORD_ARRAY_SIZE,
		GL_TEXTURE_COORD_ARRAY_TYPE,
		GL_TEXTURE_COORD_ARRAY_STRIDE,
		GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING,
		GL_FOG_COORD_ARRAY_TYPE,
		GL_FOG_COORD_ARRAY_STRIDE,
		GL_FOG_COORD_ARRAY_BUFFER_BINDING,
		GL_SECONDARY_COLOR_ARRAY_SIZE,
		GL_SECONDARY_COLOR_ARRAY_TYPE,
		GL_SECONDARY_COLOR_ARRAY_STRIDE,
		GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING
	};

	for (int i = 0; i < ARRAY_SIZE(pnames_GetIntegerv); i++) {
		int value;
		glBindVertexArray(vao[0]);
		glGetVertexArrayIntegervEXT(vao[1], pnames_GetIntegerv[i], &value);

		glBindVertexArray(vao[1]);
		pass = verify(__FUNCTION__, pnames_GetIntegerv[i], value, false, -1) && pass;
      	}

	static const GLenum pnames_IsEnabled[] = {
		GL_VERTEX_ARRAY,
		GL_COLOR_ARRAY,
		GL_EDGE_FLAG_ARRAY,
		GL_INDEX_ARRAY,
		GL_NORMAL_ARRAY,
		GL_TEXTURE_COORD_ARRAY,
		GL_FOG_COORD_ARRAY,
		GL_SECONDARY_COLOR_ARRAY
	};

	for (int i = 0; i < ARRAY_SIZE(pnames_IsEnabled); i++) {
		int value;
		glBindVertexArray(vao[0]);
		glGetVertexArrayIntegervEXT(vao[1], pnames_IsEnabled[i], &value);

		glBindVertexArray(vao[1]);
		pass = verify(__FUNCTION__, pnames_IsEnabled[i], value, false, -1) && pass;
	}

	static const GLenum pnames_GetPointerv[] = {
		GL_VERTEX_ARRAY_POINTER,
		GL_COLOR_ARRAY_POINTER,
		GL_EDGE_FLAG_ARRAY_POINTER,
		GL_INDEX_ARRAY_POINTER,
		GL_NORMAL_ARRAY_POINTER,
		GL_TEXTURE_COORD_ARRAY_POINTER,
		GL_FOG_COORD_ARRAY_POINTER,
		GL_SECONDARY_COLOR_ARRAY_POINTER
	};

	for (int i = 0; i < ARRAY_SIZE(pnames_GetPointerv); i++) {
		int value;
		glBindVertexArray(vao[0]);
		glGetVertexArrayIntegervEXT(vao[1], pnames_GetPointerv[i], &value);

		glBindVertexArray(vao[1]);
		pass = verify(__FUNCTION__, pnames_GetPointerv[i], value, true, -1) && pass;
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}


static enum piglit_result
test_GetVertexArrayIntegeri_vEXT(void* d)
{
	bool pass = true;
	/* The EXT_direct_state_access spec says:
	 *
	 *    "For GetVertexArrayIntegeri_vEXT, pname must be one of the
	 *    "Get value" tokens in tables 6.8 and 6.9 that use GetVertexAttribiv
	 *    or GetVertexAttribPointerv (so allowing only the VERTEX_ATTRIB_*
	 *    tokens) or a token of the form TEXTURE_COORD_ARRAY (the enable) or
	 *    TEXTURE_COORD_ARRAY_*"
	 */
	static const GLenum pnames_VertexAttribArray[] = {
		GL_VERTEX_ATTRIB_ARRAY_ENABLED,
		GL_VERTEX_ATTRIB_ARRAY_SIZE,
		GL_VERTEX_ATTRIB_ARRAY_STRIDE,
		GL_VERTEX_ATTRIB_ARRAY_TYPE,
		GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,
		GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING
	};

	int max_attribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attribs);
	for (int i = 0; i < ARRAY_SIZE(pnames_VertexAttribArray); i++) {
		for (int j = 0; j < max_attribs; j++) {
			int value;
			glBindVertexArray(vao[0]);
			glGetVertexArrayIntegeri_vEXT(vao[1], j, pnames_VertexAttribArray[i], &value);

			glBindVertexArray(vao[1]);
			pass = verify(__FUNCTION__, pnames_VertexAttribArray[i], value, false, j) && pass;
		}
      	}

	static const GLenum pnames_TextureCoord[] = {
		GL_TEXTURE_COORD_ARRAY,
		GL_TEXTURE_COORD_ARRAY_SIZE,
		GL_TEXTURE_COORD_ARRAY_TYPE,
		GL_TEXTURE_COORD_ARRAY_STRIDE,
		GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING,
	};
	int max_texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
	for (int i = 0; i < ARRAY_SIZE(pnames_TextureCoord); i++) {
		for (int j = 0; j < max_texture_units; j++) {
			int value;
			glBindVertexArray(vao[0]);
			glGetVertexArrayIntegeri_vEXT(vao[1], j, pnames_TextureCoord[i], &value);

			glBindVertexArray(vao[1]);
			glClientActiveTexture(GL_TEXTURE0 + j);
			pass = verify(__FUNCTION__, pnames_TextureCoord[i], value, false, -1) && pass;
			glClientActiveTexture(GL_TEXTURE0);
		}
	}

	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}


static enum piglit_result
test_GetVertexArrayPointervEXT(void* d)
{
	bool pass = true;
	/* The EXT_direct_state_access spec says:
	 *
	 *    "For GetVertexArrayPointervEXT, pname must be a *_ARRAY_POINTER token
	 *    from tables 6.6, 6.7, and 6.8 excluding VERTEX_ATTRIB_ARRAY_POINTER"
	 */
	static const GLenum pnames[] = {
		GL_VERTEX_ARRAY_POINTER,
		GL_COLOR_ARRAY_POINTER,
		GL_EDGE_FLAG_ARRAY_POINTER,
		GL_INDEX_ARRAY_POINTER,
		GL_NORMAL_ARRAY_POINTER,
		GL_TEXTURE_COORD_ARRAY_POINTER,
		GL_FOG_COORD_ARRAY_POINTER,
		GL_SECONDARY_COLOR_ARRAY_POINTER,
	};

	for (int i = 0; i < ARRAY_SIZE(pnames); i++) {
		void* value;
		glBindVertexArray(vao[0]);
		glGetVertexArrayPointervEXT(vao[1], pnames[i], &value);

		glBindVertexArray(vao[1]);
		pass = verify(__FUNCTION__, pnames[i], (GLintptr) value, true, -1) && pass;
      	}

	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}


static enum piglit_result
test_GetVertexArrayPointeri_vEXT(void* d)
{
	bool pass = true;
	/* The EXT_direct_state_access spec says:
	 *
	 *    "For GetVertexArrayPointeri_vEXT, pname must be VERTEX_ATTRIB_ARRAY_POINTER
    	 *    or TEXTURE_COORD_ARRAY_POINTE"
	 */
	int max_attribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attribs);
	for (int i = 0; i < max_attribs; i++) {
		void* value;
		glBindVertexArray(vao[0]);
		glGetVertexArrayPointeri_vEXT(vao[1], i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &value);

		glBindVertexArray(vao[1]);
		pass = verify(__FUNCTION__, GL_VERTEX_ATTRIB_ARRAY_POINTER, (GLintptr) value, true, i) && pass;
	}

	int max_texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
	for (int i = 0; i < max_texture_units; i++) {
		void* value;
		glBindVertexArray(vao[0]);
		glGetVertexArrayPointeri_vEXT(vao[1], i, GL_TEXTURE_COORD_ARRAY_POINTER, &value);

		glBindVertexArray(vao[1]);
		glClientActiveTexture(GL_TEXTURE0 + i);
		pass = verify(__FUNCTION__, GL_TEXTURE_COORD_ARRAY_POINTER, (GLintptr) value, true, -1) && pass;
		glClientActiveTexture(GL_TEXTURE0);
	}
	return (piglit_check_gl_error(GL_NO_ERROR) && pass) ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_direct_state_access");

	glGenVertexArrays(ARRAY_SIZE(vao), vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(ARRAY_SIZE(buffers), buffers);

	static const struct piglit_subtest tests[] = {
		{
			"VertexArrayVertexOffsetEXT",
			NULL,
			test_VertexArrayVertexOffsetEXT,
		},
		{
			"VertexArrayVertexOffsetEXT",
			NULL,
			test_VertexArrayVertexOffsetEXT
		},
		{
			"VertexArrayColorOffsetEXT",
			NULL,
			test_VertexArrayColorOffsetEXT
		},
		{
			"VertexArrayEdgeFlagOffsetEXT",
			NULL,
			test_VertexArrayEdgeFlagOffsetEXT
		},
		{
			"VertexArrayIndexOffsetEXT",
			NULL,
			test_VertexArrayIndexOffsetEXT
		},
		{
			"VertexArrayNormalOffsetEXT",
			NULL,
			test_VertexArrayNormalOffsetEXT
		},
		{
			"VertexArrayTexCoordOffsetEXT",
			NULL,
			test_VertexArrayTexCoordOffsetEXT
		},
		{
			"VertexArrayMultiTexCoordOffsetEXT",
			NULL,
			test_VertexArrayMultiTexCoordOffsetEXT
		},
		{
			"VertexArrayFogCoordOffsetEXT",
			NULL,
			test_VertexArrayFogCoordOffsetEXT
		},
		{
			"VertexArraySecondaryColorOffsetEXT",
			NULL,
			test_VertexArraySecondaryColorOffsetEXT
		},
		{
			"VertexArrayVertexAttribOffsetEXT",
			NULL,
			test_VertexArrayVertexAttribOffsetEXT
		},
		{
			"VertexArrayVertexAttribIOffsetEXT",
			NULL,
			test_VertexArrayVertexAttribIOffsetEXT
		},
		{
			"EnableVertexArrayEXT/DisableVertexArrayEXT",
			NULL,
			test_EnableVertexArrayEXT
		},
		{
			"GetVertexArrayIntegervEXT",
			NULL,
			test_GetVertexArrayIntegervEXT
		},
		{
			"GetVertexArrayIntegeri_vEXT",
			NULL,
			test_GetVertexArrayIntegeri_vEXT
		},
		{
			"GetVertexArrayPointervEXT",
			NULL,
			test_GetVertexArrayPointervEXT
		},
		{
			"GetVertexArrayPointeri_vEXT",
			NULL,
			test_GetVertexArrayPointeri_vEXT
		},
		{
			NULL
		}
	};

	enum piglit_result r = piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);

	glDeleteVertexArrays(ARRAY_SIZE(vao), vao);
	glDeleteBuffers(ARRAY_SIZE(buffers), buffers);

	piglit_report_result(r);
}

enum piglit_result
piglit_display(void)
{
	/* Unreachable */
	return PIGLIT_FAIL;
}
