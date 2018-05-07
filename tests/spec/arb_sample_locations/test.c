#include "piglit-util-gl.h"

/**
 * @file test.c
 *
 * This test verifies that GL_ARB_sample_locations is implemented correctly.
 * It does so by retrieving the sample positions using gl_SamplePosition,
 * interpolateAtSample() and a method described in
 * https://urldefense.proofpoint.com/v2/url?u=https-3A__mynameismjp.wordpress.com_2010_07_07_msaa-2Dsample-2Dpattern-2Ddetector_&d=DwIBAg&c=uilaK90D4TOVoH58JNXRgQ&r=Ie7_encNUsqxbSRbqbNgofw0ITcfE8JKfaUjIQhncGA&m=_I4YfQ6nzOyQ71BgwYmlUsP4O5-Pi2DfoZIGctuFnLs&s=RhpkXqf6twp4gawhoCbMx1cmqWZv4eRpX4d6JJ6_tmE&e=
 * which draws a rectangle covering each possible sample location within a pixel.
 * Each rectangle writes it's position within the pixel, which is then read
 * through a shader.
 *
 * The retrieved sample locations are then tested against expectations. This is
 * done with various MSAA levels and sample locations.
 *
 * The test can be rather slow by default, but it can be made less exhaustive by
 * passing the argument --quick.
 *
 * Sample locations are typically represented in this test as 0.4 fixed point
 * integers where 0 is the top.
 */

/* new_locations in do_test_set() needs to be expanded when this is */
#define LOG2_MAX_SAMPLES 5
#define MAX_SAMPLES (1<<LOG2_MAX_SAMPLES)

#define WIDTH 4
/* the height is 7 pixels to test pixel grid flipping */
#define HEIGHT 7

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static bool quick;

static GLuint draw_program;
static GLuint read_program;
static GLuint fb_textures[LOG2_MAX_SAMPLES+1];
static GLuint fbs[LOG2_MAX_SAMPLES+1];

static unsigned cur_fb;
static GLint grid_width, grid_height, samples;

static void
get_sample_locations(GLubyte *locations, unsigned count,
                     unsigned pixel_x, unsigned pixel_y)
{
	glViewport(pixel_x, pixel_y, 1, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, fbs[cur_fb]);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(draw_program);
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			float left = x/16.0 - (1.0/32.0);
			float bottom = (16-y)/16.0 - (1.0/32.0);
			float width = 1.0 / 16.0;
			float height = 1.0 / 16.0;
			glUniform2f(glGetUniformLocation(draw_program, "loc"), x/16.0, y/16.0);
			piglit_draw_rect(left, bottom, width, height);
		}
	}

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb_textures[cur_fb]);
	glViewport(0, 0, count, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(read_program);
	glUniform1i(glGetUniformLocation(read_program, "input"), 0);
	glUniform2i(glGetUniformLocation(read_program, "offset"), pixel_x, pixel_y);
	piglit_draw_rect(0.0, 0.0, 1.0, 1.0);

	glReadPixels(0, 0, count, 1, GL_RGBA, GL_UNSIGNED_BYTE, locations);
}

static enum piglit_result
test_pixel(const unsigned *expected, unsigned pixel_x, unsigned pixel_y)
{
	GLubyte actual_locations[32*4];
	get_sample_locations(actual_locations, samples, pixel_x, pixel_y);

	unsigned grid_x = pixel_x % grid_width;
	unsigned grid_y = pixel_y % grid_height;

	for (unsigned i = 0; i < samples; i++) {
		for (unsigned j = 0; j < 3; j++) {
			const char* src =
				(const char*[]){"Uniform", "gl_SamplePosition", "interpolateAtSample"}[j];
			unsigned x = actual_locations[i*4+j] & 0xf;
			unsigned y = actual_locations[i*4+j] >> 4;
			unsigned expected_x = expected ? expected[i*2] : 8;
			unsigned expected_y = expected ? expected[i*2+1] : 8;

			if (x != expected_x) {
			   printf("Expected X coordinate of sample %u to be %u, got %u from %s (at pixel %u, %u and grid %u, %u)\n",
			          i, expected_x, x, src, pixel_x, pixel_y, grid_x, grid_y);
			   return PIGLIT_FAIL;
			}

			if (y != expected_y) {
			   printf("Expected Y coordinate of sample %u to be %u, got %u from %s (at pixel %u, %u and grid %u, %u)\n",
			          i, expected_y, y, src, pixel_x, pixel_y, grid_x, grid_y);
			   return PIGLIT_FAIL;
			}
		}
	}

	return PIGLIT_PASS;
}

static enum piglit_result
do_test(const unsigned *locations, unsigned pixel_x, unsigned pixel_y, bool grid)
{
	if (!fbs[cur_fb]) return PIGLIT_SKIP;

	glBindFramebuffer(GL_FRAMEBUFFER, fbs[cur_fb]);

	GLint table_size;
	glGetIntegerv(GL_SAMPLES, &samples);
	glGetIntegerv(GL_SAMPLE_LOCATION_PIXEL_GRID_WIDTH_ARB, &grid_width);
	glGetIntegerv(GL_SAMPLE_LOCATION_PIXEL_GRID_HEIGHT_ARB, &grid_height);
	glGetIntegerv(GL_PROGRAMMABLE_SAMPLE_LOCATION_TABLE_SIZE_ARB, &table_size);

	unsigned grid_x = pixel_x % grid_width;
	unsigned grid_y = pixel_y % grid_height;

	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_SAMPLE_LOCATION_PIXEL_GRID_ARB, grid);
	glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_PROGRAMMABLE_SAMPLE_LOCATIONS_ARB, 1);

	for (GLint i = 0; i < table_size; i++) {
		GLfloat loc[2] = {0.5f, 0.5f};
		glFramebufferSampleLocationsfvARB(GL_FRAMEBUFFER, i, 1, loc);
	}

	for (GLint i = 0; i < samples; i++) {
		GLfloat loc[2] = {locations[i*2] / 16.0f, 1.0f - locations[i*2+1] / 16.0f};
		unsigned index = i;
		if (grid)
			index += (grid_y*grid_width+grid_x) * samples;
		glFramebufferSampleLocationsfvARB(GL_FRAMEBUFFER, index, 1, loc);
	}

	enum piglit_result result = PIGLIT_PASS;
	if (quick) {
		piglit_merge_result(&result, test_pixel(locations, grid_x, grid_y));
	} else {
		for (unsigned x = 0; x < WIDTH; x++) {
			for (unsigned y = 0; y < HEIGHT; y++) {
				const unsigned *expected = locations;
				unsigned grid_x_2 = x % grid_width;
				unsigned grid_y_2 = y % grid_height;
				if ((grid_x_2 != grid_x || grid_y_2 != grid_y) && grid)
					expected = NULL;
				piglit_merge_result(&result, test_pixel(expected, x, y));
			}
		}
	}

	return result;
}

static enum piglit_result
do_test_set(unsigned x, unsigned y, bool grid)
{
	static const unsigned new_locations[32][2] = {
		{1, 3}, {4, 1}, {2, 5}, {5, 5}, {3, 5}, {5, 2}, {1, 6}, {2, 6},
		{7, 2}, {5, 7}, {4, 8}, {2, 8}, {8, 1}, {8, 6}, {8, 3}, {9, 8},
		{2, 9}, {9, 3}, {1, 10}, {9, 10}, {10, 1}, {1, 11}, {6, 11}, {11, 6},
		{12, 5}, {10, 12}, {8, 13}, {13, 8}, {13, 12}, {6, 13}, {4, 14}, {14, 8}};
	enum piglit_result result = do_test(&new_locations[0][0], x, y, grid);

	piglit_report_subtest_result(result, "MSAA: %u, X: %u, Y: %u, Grid: %s",
	                             1<<cur_fb, x, y, grid?"true":"false");

	return result;
}

static void
create_shader_sources(char *vertex, char *fragment)
{
	static const char *fragment_exts =
		"#extension GL_ARB_gpu_shader5 : enable\n"
		"#extension GL_ARB_sample_shading : enable\n";
	static const char *fragment_main =
		"uniform vec2 loc;\n"
		"in vec2 pos;\n"
		"out vec4 color;\n"
		"float encode_location(in vec2 loc) { return (loc.x*16.0 + loc.y*256.0) / 255.0; }\n"
		"void main() {\n"
		"	color.xyz = vec3(encode_location(loc));\n";
	static const char *fragment_gl_sample_position =
		"color.y = encode_location(vec2(gl_SamplePosition.x, 1.0-gl_SamplePosition.y));\n";
	static const char *fragment_interpolate_at_sample =
		"color.z = encode_location(vec2(interpolateAtSample(pos, gl_SampleID).x, 1.0-interpolateAtSample(pos, gl_SampleID).y));\n";
	static const char *vertex_main =
		"in vec2 piglit_vertex;\n"
		"out vec2 pos;\n"
		"void main() { gl_Position = vec4(piglit_vertex*2.0-1.0, 0.0, 1.0); pos = piglit_vertex; }\n";

	int glsl_major, glsl_minor, glsl_ver;
	bool use_gl_sample_position, use_interpolate_at_sample;

	piglit_get_glsl_version(NULL, &glsl_major, &glsl_minor);
	glsl_ver = glsl_major*100 + glsl_minor;

	if (glsl_ver>=400)
		use_gl_sample_position = use_interpolate_at_sample = true;
	if (piglit_is_extension_supported("GL_ARB_gpu_shader5"))
		use_interpolate_at_sample = true;
	if (piglit_is_extension_supported("GL_ARB_sample_shading"))
		use_gl_sample_position = true;

	sprintf(fragment, "#version %u\n%s", glsl_ver, fragment_exts);
	strcat(fragment, fragment_main);
	if (use_gl_sample_position)
		strcat(fragment, fragment_gl_sample_position);
	if (use_interpolate_at_sample)
		strcat(fragment, fragment_interpolate_at_sample);
	strcat(fragment, "}");

	sprintf(vertex, "#version %u\n%s", glsl_ver, fragment_exts);
	strcat(vertex, vertex_main);
}

void
piglit_init(int argc, char **argv)
{
	char vertex_source[1024];
	char fragment_source[1024];
	piglit_require_extension("GL_ARB_sample_locations");

	if (argc > 1 && strcmp(argv[1], "--quick") == 0)
		quick = true;

	create_shader_sources(vertex_source, fragment_source);

	draw_program = piglit_build_simple_program(vertex_source, fragment_source);

	read_program = piglit_build_simple_program(
			"#version 150\n"
			"in vec2 piglit_vertex;\n"
			"void main() { gl_Position = vec4(piglit_vertex*2.0-1.0, 0.0, 1.0); }\n",
			"#version 150\n"
			"uniform sampler2DMS tex;\n"
			"uniform ivec2 offset;\n"
			"out vec4 color;\n"
			"void main() { color = texelFetch(tex, offset, int(gl_FragCoord.x)); }\n");

	GLint count;
	glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &count);
	if (count > MAX_SAMPLES)
		count = MAX_SAMPLES;
	count = log2u(count) + 1;

	glGenTextures(count, fb_textures);
	glGenFramebuffers(count, fbs);
	for (cur_fb = 0; cur_fb < count; cur_fb++) {
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb_textures[cur_fb]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1<<cur_fb, GL_RGBA8, WIDTH, HEIGHT, GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, fbs[cur_fb]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fb_textures[cur_fb], 0);
	}
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	for (cur_fb = 0; cur_fb < sizeof(fb_textures)/sizeof(fb_textures[0]); cur_fb++) {
		for (unsigned x = 0; x < WIDTH; x++) {
			for (unsigned y = 0; y < HEIGHT; y++) {
				piglit_merge_result(&result, do_test_set(x, y, false));
				piglit_merge_result(&result, do_test_set(x, y, true));
			}
		}
	}

	piglit_present_results();

	return result;
}
