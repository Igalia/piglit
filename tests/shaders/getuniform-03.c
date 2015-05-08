/*
 * Copyright 2015 Intel Corporation
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

PIGLIT_GL_TEST_CONFIG_END

struct builtin_uniform_t {
	const char *name;
	GLenum type;
	bool is_array;
	bool found;
} uniforms[] = {
	{ "gl_DepthRange.near", GL_FLOAT, false , false },
	{ "gl_DepthRange.far", GL_FLOAT, false , false },
	{ "gl_DepthRange.diff", GL_FLOAT, false , false },
	/* { "gl_NumSamples", GL_INT, false , false }, requires OGL 4.0 */
	{ "gl_ModelViewMatrix", GL_FLOAT_MAT4, false , false },
	{ "gl_ProjectionMatrix", GL_FLOAT_MAT4, false , false  },
	{ "gl_ModelViewProjectionMatrix", GL_FLOAT_MAT4, false , false },
	{ "gl_TextureMatrix", GL_FLOAT_MAT4, true , false  },
	{ "gl_NormalMatrix", GL_FLOAT_MAT3, false , false  },
	{ "gl_ModelViewMatrixInverse", GL_FLOAT_MAT4, false , false },
	{ "gl_ProjectionMatrixInverse", GL_FLOAT_MAT4, false , false  },
	{ "gl_TextureMatrixInverse", GL_FLOAT_MAT4, true , false  },
	{ "gl_ModelViewMatrixTranspose", GL_FLOAT_MAT4, false , false  },
	{ "gl_ProjectionMatrixTranspose", GL_FLOAT_MAT4, false , false },
	{ "gl_ModelViewProjectionMatrixTranspose", GL_FLOAT_MAT4, false , false },
	{ "gl_TextureMatrixTranspose", GL_FLOAT_MAT4, true , false  },
	{ "gl_ModelViewMatrixInverseTranspose", GL_FLOAT_MAT4, false , false  },
	{ "gl_ProjectionMatrixInverseTranspose", GL_FLOAT_MAT4, false , false },
	{ "gl_ModelViewProjectionMatrixInverseTranspose", GL_FLOAT_MAT4, false  },
	{ "gl_TextureMatrixInverseTranspose", GL_FLOAT_MAT4, true , false },
	{ "gl_NormalScale", GL_FLOAT, false , false },
	{ "gl_ClipPlane", GL_FLOAT_VEC4, true , false },
	{ "gl_Point.size", GL_FLOAT, false , false },
	{ "gl_Point.sizeMin", GL_FLOAT, false , false },
	{ "gl_Point.sizeMax", GL_FLOAT, false , false },
	{ "gl_Point.fadeThresholdSize", GL_FLOAT, false , false },
	{ "gl_Point.distanceConstantAttenuation", GL_FLOAT, false , false },
	{ "gl_Point.distanceLinearAttenuation", GL_FLOAT, false , false },
	{ "gl_Point.distanceQuadraticAttenuation", GL_FLOAT, false , false },
	{ "gl_FrontMaterial.emission", GL_FLOAT_VEC4, false , false },
	{ "gl_FrontMaterial.ambient", GL_FLOAT_VEC4, false , false },
	{ "gl_FrontMaterial.diffuse", GL_FLOAT_VEC4, false , false },
	{ "gl_FrontMaterial.specular", GL_FLOAT_VEC4, false , false },
	{ "gl_FrontMaterial.shininess", GL_FLOAT, false , false },
	{ "gl_BackMaterial.emission", GL_FLOAT_VEC4, false , false },
	{ "gl_BackMaterial.ambient", GL_FLOAT_VEC4, false , false },
	{ "gl_BackMaterial.diffuse", GL_FLOAT_VEC4, false , false },
	{ "gl_BackMaterial.specular", GL_FLOAT_VEC4, false , false },
	{ "gl_BackMaterial.shininess", GL_FLOAT, false , false },
	{ "gl_LightSource[0].ambient", GL_FLOAT_VEC4, false , false },
	{ "gl_LightSource[0].diffuse", GL_FLOAT_VEC4, false , false },
	{ "gl_LightSource[0].specular", GL_FLOAT_VEC4, false , false },
	{ "gl_LightSource[0].position", GL_FLOAT_VEC4, false , false },
	{ "gl_LightSource[0].halfVector", GL_FLOAT_VEC4, false , false },
	{ "gl_LightSource[0].spotDirection", GL_FLOAT_VEC3, false , false },
	{ "gl_LightSource[0].spotExponent", GL_FLOAT, false , false },
	{ "gl_LightSource[0].spotCutoff", GL_FLOAT, false , false },
	{ "gl_LightSource[0].spotCosCutoff", GL_FLOAT, false , false },
	{ "gl_LightSource[0].constantAttenuation", GL_FLOAT, false , false },
	{ "gl_LightSource[0].linearAttenuation", GL_FLOAT, false , false },
	{ "gl_LightSource[0].quadraticAttenuation", GL_FLOAT, false , false },
	{ "gl_LightModel.ambient", GL_FLOAT_VEC4, false , false },
	{ "gl_FrontLightModelProduct.sceneColor", GL_FLOAT_VEC4, false , false },
	{ "gl_BackLightModelProduct.sceneColor", GL_FLOAT_VEC4, false , false },
	{ "gl_FrontLightProduct[0].ambient", GL_FLOAT_VEC4, true , false },
	{ "gl_FrontLightProduct[0].diffuse", GL_FLOAT_VEC4, true , false },
	{ "gl_FrontLightProduct[0].specular", GL_FLOAT_VEC4, true , false },
	{ "gl_BackLightProduct[0].ambient", GL_FLOAT_VEC4, true , false },
	{ "gl_BackLightProduct[0].diffuse", GL_FLOAT_VEC4, true , false },
	{ "gl_BackLightProduct[0].specular", GL_FLOAT_VEC4, true , false },
	{ "gl_TextureEnvColor", GL_FLOAT_VEC4, true , false },
	{ "gl_EyePlaneS", GL_FLOAT_VEC4, true , false },
	{ "gl_EyePlaneT", GL_FLOAT_VEC4, true , false },
	{ "gl_EyePlaneR", GL_FLOAT_VEC4, true , false },
	{ "gl_EyePlaneQ", GL_FLOAT_VEC4, true , false },
	{ "gl_ObjectPlaneS", GL_FLOAT_VEC4, true , false },
	{ "gl_ObjectPlaneT", GL_FLOAT_VEC4, true , false },
	{ "gl_ObjectPlaneR", GL_FLOAT_VEC4, true , false },
	{ "gl_ObjectPlaneQ", GL_FLOAT_VEC4, true , false },
	{ "gl_Fog.color", GL_FLOAT_VEC4, false , false },
	{ "gl_Fog.density", GL_FLOAT, false , false },
	{ "gl_Fog.start", GL_FLOAT, false , false },
	{ "gl_Fog.end", GL_FLOAT, false , false },
	{ "gl_Fog.scale", GL_FLOAT, false , false },
};

static const char vs_header[] =
   "void main()\n"
   "{\n"
   "  gl_Position = vec4(1);\n";

static const char vs_footer[] =
   "}\n";

char *
gen_vs_shader_all()
{
	/* strlen("  gl_Position += vec4(gl_ModelViewProjectionMatrixInverse
	 *        "Transpose[0]);\n") */
	size_t max_line_size = 64;
	size_t uniforms_count = ARRAY_SIZE(uniforms);
	size_t shader_len = strlen(vs_header) +
			    uniforms_count * max_line_size +
			    strlen(vs_footer) + 1;
	size_t i;

	/* allocate the shader */
	char *vs_text = malloc(shader_len);
	memset(vs_text, 0, shader_len);

	/* add the header */
	strcat(vs_text, vs_header);

	/* add a reference to every uniform */
	for (i = 0; i < uniforms_count; i++) {
		strcat(vs_text, "  gl_Position += ");
		switch(uniforms[i].type) {
		case GL_FLOAT_VEC3:
			strcat(vs_text, "vec4(");
			strcat(vs_text, uniforms[i].name);
			strcat(vs_text, ", 1)");
			break;
		case GL_FLOAT_MAT4:
			strcat(vs_text, "vec4(");
			strcat(vs_text, uniforms[i].name);
			strcat(vs_text, "[0])");
			break;
		case GL_FLOAT_MAT3:
			strcat(vs_text, "vec4(");
			strcat(vs_text, uniforms[i].name);
			strcat(vs_text, ")");
			break;
		default:
			strcat(vs_text, uniforms[i].name);
			if (uniforms[i].is_array)
				strcat(vs_text, "[0]");
		}
		strcat(vs_text, ";\n");
	}

	/* add the footer */
	strcat(vs_text, vs_footer);

	/* Uncomment the following to view the generated shader */
	/*fprintf(stderr, "shader = '%s'\n", vs_text);*/

	return vs_text;
}

enum piglit_result
piglit_display(void)
{
	/* never called */
	return PIGLIT_FAIL;
}

static struct builtin_uniform_t *
find_uniform(const char *name)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(uniforms); i++) {
		/* OpenGL ES 3.0 and OpenGL 4.2 require that the "[0]" be
		 * appended to the name.  Earlier versions of the spec are
		 * ambiguous.  Accept either name.
		*/
		if (strcmp(uniforms[i].name, name) == 0 ||
		    (strcmp(name + strlen(name) - 3, "[0]") == 0 &&
		     strncmp(uniforms[i].name, name, strlen(name) - 3) == 0))
			return &uniforms[i];
	}

	return NULL;
}

void
piglit_init(int argc, char **argv)
{
	GLint numUniforms, i;
	bool pass = true;
	GLuint vs, prog;
	char *vs_text;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	vs_text = gen_vs_shader_all();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	prog = piglit_link_simple_program(vs, 0);
	if (!prog) {
		printf("Compilation error. Aborting...\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);
	free(vs_text);

	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &numUniforms);
	if (numUniforms != ARRAY_SIZE(uniforms)) {
		printf("Unexpected number of uniforms (found %d, expected "
		       "%lu)\n", numUniforms, ARRAY_SIZE(uniforms));
	}

	/* check the types of the active uniforms and check which ones got
	 * references by glGetActiveUniform for later comparaison. */
	for (i = 0; i < numUniforms; i++) {
		struct builtin_uniform_t *uniform;
		GLcharARB name[100];
		GLsizei len;
		GLint size;
		GLenum type;

		glGetActiveUniform(prog, i, sizeof(name), &len, &size, &type,
				   name);

		uniform = find_uniform(name);

		if (!uniform) {
			fprintf(stderr, "Cannot find uniform '%s'\n", name);
			pass = false;
			continue;
		}

		uniform->found = true;

		if (type != uniform->type) {
			printf("Wrong type for '%s' (found %s(0x%x)), "
			       "expected %s(0x%x))\n",
				name, piglit_get_gl_enum_name(type), type,
				piglit_get_gl_enum_name(uniform->type),
				uniform->type);
				pass = false;
		}
	}

	/* check that no uniform got forgotten as there is the possibility that
	 * one got referenced twice!
	 */
	for (i = 0; i < ARRAY_SIZE(uniforms); i++) {
		if (uniforms[i].found == false) {
			fprintf(stderr, "uniform '%s' is missing from the "
					"active uniform list!\n",
				uniforms[i].name);

			/* A missing builtin is legal, as stated by page 80
			 * (page 94 of the PDF) of the OpenGL 2.1 spec:
			 *
			 * "The returned uniform name can be the name of
			 * built-in uniform state as well."
			 */

			/* FIXME: verify that the missing uniform as not been
			 * forgotten by the implementation. One way could have
			 * been to count the number of components used by the
			 * uniforms and add more components until reaching
			 * shader then adding other uniforms to reach
			 * GL_MAX_VERTEX_UNIFORM_COMPONENTS. If the shader still
			 * compiles, and assuming that
			 * GL_MAX_VERTEX_UNIFORM_COMPONENTS reports a good value
			 * then we can assume that the compiler just replaced
			 * the uniform with something else. If it does not
			 * compile, then it probably lied somewhere!
			 *
			 * The problem with this approach is that counting the
			 * number of components used for some types such as
			 * matrices is implementation-dependent...
			 *
			 * Until we have such a way to verify, let's not fail
			 * the test!
			 */
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
