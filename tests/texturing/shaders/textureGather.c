#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum { NOSTAGE, VS, FS } stage = NOSTAGE;
enum { NONE = -1, RED, GREEN, BLUE, ALPHA, ZERO, ONE } swizzle = NONE;
enum { UNORM_T, FLOAT_T, INT_T, UINT_T, SHADOW_T, NUM_COMPTYPES } comptype = UNORM_T;
enum { SAMPLER_2D, SAMPLER_2DARRAY, SAMPLER_CUBE, SAMPLER_CUBEARRAY, SAMPLER_2DRECT } sampler = SAMPLER_2D;
bool use_offset = false;
bool use_nonconst = false;
bool use_offsets = false;
int components = 0;
int comp_select = -1;

int min_offset = 0;
int max_offset = 0;

int texture_width = 32;
int texture_height = 32;

GLenum internalformat_for_components[][4] = {
	{ GL_R16, GL_RG16, GL_RGB16, GL_RGBA16, },
	{ GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F, },
	{ GL_R16I, GL_RG16I, GL_RGB16I, GL_RGBA16I },
	{ GL_R16UI, GL_RG16UI, GL_RGB16UI, GL_RGBA16UI },
	{ GL_DEPTH_COMPONENT, 0, 0, 0 },
};
GLenum format_for_components[][4] = {
	{ GL_RED, GL_RG, GL_RGB, GL_RGBA },
	{ GL_RED, GL_RG, GL_RGB, GL_RGBA },
	{ GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER,  GL_RGBA_INTEGER },
	{ GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER,  GL_RGBA_INTEGER },
	{ GL_DEPTH_COMPONENT, 0, 0, 0 },
};
GLenum swizzles[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_ZERO, GL_ONE };
int slices_for_sampler[] = { 1, 3, 6, 12, 1 };
GLenum target_for_sampler[] = { GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_RECTANGLE };
GLenum address_mode = GL_REPEAT;

unsigned char *pixels;
float *expected;

enum piglit_result
piglit_display(void)
{
	int i, j;
	bool pass = true;

	glViewport(0, 0, texture_width, texture_height);
	glClearColor(0.4, 0.4, 0.4, 0.4);
	glClear(GL_COLOR_BUFFER_BIT);

	if (swizzle >= 0) {
		GLint sw[] = { GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO };
		if (comp_select != -1)
			sw[comp_select] = swizzles[swizzle];
		else
			sw[0] = swizzles[swizzle];
		glTexParameteriv(target_for_sampler[sampler], GL_TEXTURE_SWIZZLE_RGBA, sw);
	}

	if (stage == FS)
		glDrawArrays(GL_TRIANGLES, 0, 6);
	else
		glDrawArrays(GL_POINTS, 0, texture_width * texture_height);

	for (j = 1; j < texture_height - 1; j++)
		for (i = 1; i < texture_width - 1; i++) {
			float *pe = &expected[4 * (j * texture_width + i)];
			pass = piglit_probe_pixel_rgba(i, j, pe) && pass;
		}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

/* TODO:
 * Test other sampler types: gsampler2D|gsampler2DArray|gsamplerCube|gsamplerCubeArray
 * Test GS texturing too -- Paul?
 */

static unsigned char
pixel_value(int i, int j, int offset_sel)
{
	if (swizzle == ZERO)
		return 0;
	if (swizzle == ONE)
		return 255;

	if (use_offset) {
		/* apply texel offset */
		i += min_offset;
		j += max_offset;
	} else if (use_offsets) {
		switch (offset_sel) {
		case 0:
			i += min_offset;
			j += max_offset;
			break;
		case 1:
			i += max_offset;
			j += min_offset;
			break;
		case 2:
			i += 3;
			j += 3;
			break;
		case 3:
			i += -3;
			j += -3;
			break;
		}
	}

	if (address_mode == GL_REPEAT) {
		/* WRAP at border */
		i += texture_width;
		j += texture_height;
		i %= texture_width;
		j %= texture_height;
	}
	else if (address_mode == GL_CLAMP_TO_EDGE) {
		if (i < 0) i = 0;
		if (j < 0) j = 0;
		if (i > texture_width - 1) i = texture_width - 1;
		if (j > texture_height - 1) j = texture_height - 1;
	}

	return i + j * texture_width;
}

static float
norm_value(int x)
{
	return (float)x / 255.0f;
}

static void
make_image(int num_channels, int use_channel)
{
	unsigned char *pp = pixels;
	int i, j, ch;

	for (j = 0; j < texture_height; j++)
		for (i = 0; i < texture_width; i++)
			for (ch = 0; ch < num_channels; ch++)
				*pp++ = (ch == use_channel) ? (i+j*texture_width) : 128;
}

static float shadow_compare(float x)
{
	return x > 0.5f ? 1.0f : 0.0f;
}

static void
make_expected(void)
{
	float *pe = expected;
	int i, j;

	for (j = 0; j < texture_height; j++)
		for (i = 0; i < texture_width; i++) {
			if (comptype == SHADOW_T) {
				if (use_offsets) {
					*pe++ = shadow_compare(norm_value(pixel_value(i, j, 0)));
					*pe++ = shadow_compare(norm_value(pixel_value(i, j, 1)));
					*pe++ = shadow_compare(norm_value(pixel_value(i, j, 2)));
					*pe++ = shadow_compare(norm_value(pixel_value(i, j, 3)));
				} else {
					*pe++ = shadow_compare(norm_value(pixel_value(i, j + 1, 0)));
					*pe++ = shadow_compare(norm_value(pixel_value(i + 1, j + 1, 0)));
					*pe++ = shadow_compare(norm_value(pixel_value(i + 1, j, 0)));
					*pe++ = shadow_compare(norm_value(pixel_value(i, j, 0)));
				}
			}
			else {
				if (use_offsets) {
					*pe++ = norm_value(pixel_value(i, j, 0));
					*pe++ = norm_value(pixel_value(i, j, 1));
					*pe++ = norm_value(pixel_value(i, j, 2));
					*pe++ = norm_value(pixel_value(i, j, 3));
				} else {
					*pe++ = norm_value(pixel_value(i, j + 1, 0));
					*pe++ = norm_value(pixel_value(i + 1, j + 1, 0));
					*pe++ = norm_value(pixel_value(i + 1, j, 0));
					*pe++ = norm_value(pixel_value(i, j, 0));
				}
			}
		}
}

static void
upload_verts(void)
{
	if (stage == VS) {
		size_t size = 4 * texture_width * texture_height * sizeof(float);
		float *v = (float *)malloc(size);
		float *pv = v;
		int i, j;
		for (j = 0; j < texture_height; j++)
			for (i = 0; i < texture_width; i++) {
				*pv++ = (i + 0.5f) * 2 / texture_width - 1;
				*pv++ = (j + 0.5f) * 2 / texture_height - 1;
				*pv++ = 0;
				*pv++ = 1;
			}
		glBufferData(GL_ARRAY_BUFFER, size, v, GL_STATIC_DRAW);
		free(v);
	}
	else {
		static const float verts[] = {
			-1, -1, 0, 1,
			-1, 1, 0, 1,
			1, 1, 0, 1,
			-1, -1, 0, 1,
			1, 1, 0, 1,
			1, -1, 0, 1,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	}
}

void
do_requires(void)
{
	int max_components;
	piglit_require_GLSL_version(130);
	piglit_require_extension("GL_ARB_texture_gather");

	/* check whether component count will actually work */
	glGetIntegerv(GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB, &max_components);
	if (components > max_components) {
		printf("Test requires gather from texture with %d components;"
		       "This implementation only supports %d\n",
		       components, max_components);
		piglit_report_result(PIGLIT_SKIP);
	}

        /* get the offset limits */
        glGetIntegerv(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB, &min_offset);
        glGetIntegerv(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB, &max_offset);

        /* increase width/height if necessary */
        if (use_offset || use_offsets) {
           texture_width = MAX2(texture_width, (max_offset + 1) * 2);
           texture_height = MAX2(texture_height, (max_offset + 1) * 2);
        }

	/* if we are trying to swizzle, check that we can! */
	if (swizzle != -1)
		piglit_require_extension("GL_EXT_texture_swizzle");

	/* check the sampler type we want actually exists */
	if (sampler == SAMPLER_CUBEARRAY)
		piglit_require_extension("GL_ARB_texture_cube_map_array");

	if ((use_offsets || use_offset) && (sampler == SAMPLER_CUBE || sampler == SAMPLER_CUBEARRAY)) {
		printf("Offset is not supported with cube or cube array samplers.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (comptype == SHADOW_T && components > 1) {
		printf("Shadow supported with single-component textures only\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (comptype == SHADOW_T && comp_select != -1) {
		printf("Shadow not supported with component select parameter\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* if we are trying to specify the component from the shader,
	 * or use non-constant offsets, or use shadow comparitor, or
	 * use gsampler2DRect, check that we have ARB_gpu_shader5
	 */
	if (comp_select != -1 || use_offsets || use_nonconst || comptype == SHADOW_T || sampler == SAMPLER_2DRECT)
		piglit_require_extension("GL_ARB_gpu_shader5");

	/* if rect sampler, repeat is not available */
	if (sampler == SAMPLER_2DRECT && address_mode == GL_REPEAT) {
		printf("GL_REPEAT not supported with rectangle textures\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

static void
upload_2d(GLenum target, void *pixels)
{
	glTexImage2D(target, 0,
		     internalformat_for_components[comptype][components - 1],
		     texture_width, texture_height,
		     0, format_for_components[comptype][components-1],
		     GL_UNSIGNED_BYTE, pixels);
}

static void
upload_array_slice(GLenum target, int slice, void *pixels)
{
	glTexSubImage3D(target, 0, 0, 0, slice, texture_width, texture_height, 1,
			format_for_components[comptype][components-1],
			GL_UNSIGNED_BYTE, pixels);
}

static void
upload_3d(GLenum target, void *pixels)
{
	glTexImage3D(target, 0,
		     internalformat_for_components[comptype][components - 1],
		     texture_width, texture_height,
		     slices_for_sampler[sampler], 0,
		     format_for_components[comptype][components-1],
		     GL_UNSIGNED_BYTE, pixels);
}

static int
channel_to_fill(void) {
	if (swizzle != NONE)
		return swizzle;
	if (comp_select != NONE)
		return comp_select;
	return 0;
}

static void
do_texture_setup(void)
{
	GLuint tex;
	GLenum target = target_for_sampler[sampler];
	pixels = malloc(components * sizeof(unsigned char) * texture_width * texture_height);
	expected = malloc(4 * sizeof(float) * texture_width * texture_height);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	make_image(components, channel_to_fill());
	make_expected();

	switch(sampler) {
	case SAMPLER_2D:
	case SAMPLER_2DRECT:
		upload_2d(target, pixels);
		break;
	case SAMPLER_2DARRAY:
		upload_3d(target, NULL);
		upload_array_slice(target, 1, pixels);
		break;
	case SAMPLER_CUBE:
		/* legacy cubes are weird. the only sane way to specify the whole
		 * thing at once is using glTexStorage, and we'd rather not rely on
		 * ARB_texture_storage just for that. */
		upload_2d(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, NULL);
		upload_2d(GL_TEXTURE_CUBE_MAP_POSITIVE_X, NULL);
		upload_2d(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, NULL);
		upload_2d(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, NULL);
		upload_2d(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, NULL);
		upload_2d(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, pixels);
		break;
	case SAMPLER_CUBEARRAY:
		upload_3d(target, NULL);
		upload_array_slice(target, 10, pixels);
		break;
	}

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (comptype == SHADOW_T) {
		glTexParameteri(target_for_sampler[sampler], GL_TEXTURE_COMPARE_MODE,
				GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(target_for_sampler[sampler], GL_TEXTURE_COMPARE_FUNC,
				GL_LESS);
	}

	glTexParameteri(target_for_sampler[sampler], GL_TEXTURE_WRAP_S, address_mode);
	glTexParameteri(target_for_sampler[sampler], GL_TEXTURE_WRAP_T, address_mode);
	glTexParameteri(target_for_sampler[sampler], GL_TEXTURE_WRAP_R, address_mode);
}

static void
do_shader_setup(void)
{
	GLint prog;
	GLint sampler_loc, offset_loc;
	char *vs_code, *fs_code;
	char *offset_coords;
	char *prefix[] = { "" /* unorm */, "" /* float */, "i" /* int */, "u" /* uint */, "" /* shadow */ };
	char *scale[] = {
		"vec4(1)",		/* unorm + GL_ONE swizzle */
		"vec4(1)",		/* float */
		"vec4(1.0/255.0)",	/* int */
		"vec4(1.0/255.0)",	/* uint */
		"vec4(1)",		/* shadow */
	};
	char *samplersuffix[] = { "2D", "2DArray", "Cube", "CubeArray", "2DRect" };
	char *vs_tc_expr[] = {
		"0.5 * pos.xy + vec2(0.5)",
		"vec3(0.5 * pos.xy + vec2(0.5), 1)",
		"vec3(pos.x, -pos.y, 1)",		/* cube */
		"vec4(pos.x, -pos.y, 1, 1)",		/* cube array */
		"textureSize(s).xy * (0.5 * pos.xy + vec2(0.5))",		/* 2drect */
	};
	char *fs_tc_expr[] = {
		"gl_FragCoord.xy / textureSize(s, 0).xy",
		"vec3(gl_FragCoord.xy / textureSize(s, 0).xy, 1)",
		"vec3(vec2(2, -2) * (gl_FragCoord.xy / textureSize(s, 0).xy - vec2(0.5)), 1)",	/* cube */
		"vec4(vec2(2, -2) * (gl_FragCoord.xy / textureSize(s, 0).xy - vec2(0.5)), 1, 1)",	/* cube array */
		"gl_FragCoord.xy",		/* 2drect */
	};
	char *comp_expr[] = {"", ", 0", ", 1", ", 2", ", 3"};
	bool need_shader5 = (comp_select != -1) || use_offsets || use_nonconst || (comptype == SHADOW_T) || sampler == SAMPLER_2DRECT;

	if (use_offsets)
		asprintf(&offset_coords, "const ivec2 osets[4] = ivec2[4](ivec2(%d, %d), ivec2(%d, %d), ivec2(3, 3), ivec2(-3, -3));\n",
			 min_offset, max_offset, max_offset, min_offset);
	else if (use_offset)
		asprintf(&offset_coords, ", ivec2(%d,%d)", min_offset, max_offset);

	if (stage == VS) {
		asprintf(&vs_code, "#version %s\n"
				"#extension GL_ARB_explicit_attrib_location: require\n"
				"#extension GL_ARB_texture_gather: require\n"
				"%s"
				"%s"
				"\n"
				"layout(location=0) in vec4 pos;\n"
				"uniform %ssampler%s%s s;\n"
				"%s"
				"out vec4 c;\n"
				"\n"
				"void main() {\n"
				"	gl_Position = pos;\n"
				"	c = %s * textureGather%s(s, %s %s %s %s);\n"
				"}\n",
				need_shader5 ? "150" : "130",
				sampler == SAMPLER_CUBEARRAY ? "#extension GL_ARB_texture_cube_map_array: require\n" : "",
				need_shader5 ? "#extension GL_ARB_gpu_shader5: require\n" : "",
				prefix[comptype],
				samplersuffix[sampler],
				comptype == SHADOW_T ? "Shadow" : "",
				use_offsets ? offset_coords :use_nonconst ? "uniform ivec2 o1,o2;\n" : "",
				swizzle == ONE ? scale[0] : scale[comptype],
				use_offsets ? "Offsets" : (use_offset ? "Offset" : ""),
				vs_tc_expr[sampler],
				comptype == SHADOW_T ? ", 0.5" : "",
				use_offsets ? ", osets" : use_nonconst ? ", o1+o2" : use_offset ? offset_coords :  "",
				comp_expr[1 + comp_select]);
		asprintf(&fs_code,
				"#version %s\n"
				"\n"
				"in vec4 c;\n"
				"\n"
				"void main() {\n"
				"	gl_FragColor = c;\n"
				"}\n",
				need_shader5 ? "150" : "130");
	}
	else {
		asprintf(&vs_code,
				"#version %s\n"
				"#extension GL_ARB_explicit_attrib_location: require\n"
				"layout(location=0) in vec4 pos;\n"
				"\n"
				"void main() {\n"
				"	gl_Position = pos;\n"
				"}\n",
				need_shader5 ? "150" : "130");
		asprintf(&fs_code,
				"#version %s\n"
				"#extension GL_ARB_texture_gather: require\n"
				"%s"
				"%s"
				"\n"
				"uniform %ssampler%s%s s;\n"
				"%s"
				"\n"
				"void main() {\n"
				"	gl_FragColor = %s * textureGather%s(s, %s %s %s %s);\n"
				"}\n",
				need_shader5 ? "150" : "130",
				sampler == SAMPLER_CUBEARRAY ? "#extension GL_ARB_texture_cube_map_array: require\n" : "",
				need_shader5 ? "#extension GL_ARB_gpu_shader5: require\n" : "",
				prefix[comptype],
				samplersuffix[sampler],
				comptype == SHADOW_T ? "Shadow" : "",
				use_offsets ? offset_coords :use_nonconst ? "uniform ivec2 o1,o2;\n" : "",
				swizzle == ONE ? scale[0] : scale[comptype],
				use_offsets ? "Offsets" : (use_offset ? "Offset" : ""),
				fs_tc_expr[sampler],
				comptype == SHADOW_T ? ", 0.5" : "",
				use_offsets ? ", osets" : use_nonconst ? ", o1+o2" : use_offset ? offset_coords :  "",
				comp_expr[1 + comp_select]);
	}

	prog = piglit_build_simple_program(vs_code, fs_code);

	glUseProgram(prog);
	sampler_loc = glGetUniformLocation(prog, "s");
	glUniform1i(sampler_loc, 0);

	if (use_nonconst) {
		offset_loc = glGetUniformLocation(prog, "o1");
		glUniform2i(offset_loc, min_offset, 0);
		offset_loc = glGetUniformLocation(prog, "o2");
		glUniform2i(offset_loc, 0, max_offset);
	}
}

static void
do_geometry_setup(void)
{
	GLuint vbo;
	if (piglit_get_gl_version() >= 31) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	upload_verts();
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void
fail_with_usage(void)
{
	printf("Usage: textureGather <stage> [offset] [nonconst] [offsets] <components> <swizzle> <comptype> <sampler> <compselect> <addressmode>\n"
	       "	stage = vs|fs\n"
	       "	components = r|rg|rgb|rgba\n"
	       "	swizzle = red|green|blue|alpha|zero|one\n"
	       "	comptype = unorm|float|uint|int|shadow\n"
	       "	sampler = 2D|2DArray|Cube|CubeArray|2DRect\n"
	       "	compselect = 0|1|2|3\n"
	       "	addressmode = repeat|clamp\n");
	piglit_report_result(PIGLIT_SKIP);
}

void
piglit_init(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++) {
		char *opt = argv[i];
		if (!strcmp(opt, "vs")) stage = VS;
		else if (!strcmp(opt, "fs")) stage = FS;
		else if (!strcmp(opt, "offset")) use_offset = true;
		else if (!strcmp(opt, "nonconst")) use_nonconst = true;
		else if (!strcmp(opt, "offsets")) use_offsets = true;
		else if (!strcmp(opt, "r")) components = 1;
		else if (!strcmp(opt, "rg")) components = 2;
		else if (!strcmp(opt, "rgb")) components = 3;
		else if (!strcmp(opt, "rgba")) components = 4;
		else if (!strcmp(opt, "red")) swizzle = 0;
		else if (!strcmp(opt, "green")) swizzle = 1;
		else if (!strcmp(opt, "blue")) swizzle = 2;
		else if (!strcmp(opt, "alpha")) swizzle = 3;
		else if (!strcmp(opt, "zero")) swizzle = 4;
		else if (!strcmp(opt, "one")) swizzle = 5;
		else if (!strcmp(opt, "unorm")) comptype = UNORM_T;
		else if (!strcmp(opt, "float")) comptype = FLOAT_T;
		else if (!strcmp(opt, "int")) comptype = INT_T;
		else if (!strcmp(opt, "uint")) comptype = UINT_T;
		else if (!strcmp(opt, "shadow")) comptype = SHADOW_T;
		else if (!strcmp(opt, "2D")) sampler = SAMPLER_2D;
		else if (!strcmp(opt, "2DArray")) sampler = SAMPLER_2DARRAY;
		else if (!strcmp(opt, "Cube")) sampler = SAMPLER_CUBE;
		else if (!strcmp(opt, "CubeArray")) sampler = SAMPLER_CUBEARRAY;
		else if (!strcmp(opt, "2DRect")) sampler = SAMPLER_2DRECT;
		else if (!strcmp(opt, "0")) comp_select = 0;
		else if (!strcmp(opt, "1")) comp_select = 1;
		else if (!strcmp(opt, "2")) comp_select = 2;
		else if (!strcmp(opt, "3")) comp_select = 3;
		else if (!strcmp(opt, "repeat")) address_mode = GL_REPEAT;
		else if (!strcmp(opt, "clamp")) address_mode = GL_CLAMP_TO_EDGE;
	}

	if (stage == NOSTAGE) fail_with_usage();
	if (components == 0) fail_with_usage();

	if (use_nonconst) use_offset = true;

	do_requires();
	do_texture_setup();

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error in texture setup\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	do_shader_setup();

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error in shader setup\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	do_geometry_setup();

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error in geometry setup\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}
