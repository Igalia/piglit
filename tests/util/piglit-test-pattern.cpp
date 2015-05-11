/*
 * Copyright © 2013 Intel Corporation
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
 * \file piglit-test-pattern.cpp
 *
 * This file defines the functions which can be utilized to draw test patterns
 * in to color, depth or stencil buffers.
 *
 */
#include "piglit-test-pattern.h"
using namespace piglit_util_test_pattern;

const float TestPattern::no_projection[4][4] = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};


void Triangles::compile()
{
	/* Triangle coords within (-1,-1) to (1,1) rect */
	static const float pos_within_tri[][2] = {
		{ -0.5, -1.0 },
		{  0.0,  1.0 },
		{  0.5, -1.0 }
	};

	/* Number of triangle instances across (and down) */
	int tris_across = 8;

	/* Total number of triangles drawn */
	num_tris = tris_across * tris_across;

	/* Scaling factor uniformly applied to triangle coords */
	float tri_scale = 0.8 / tris_across;

	/* Amount each triangle should be rotated compared to prev */
	float rotation_delta = M_PI * 2.0 / num_tris;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos_within_tri;\n"
		"uniform float tri_scale;\n"
		"uniform float rotation_delta;\n"
		"uniform int tris_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int tri_num; /* [0, num_tris) */\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = tri_scale * pos_within_tri;\n"
		"  float rotation = rotation_delta * tri_num;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  int i = int(mod(float(tri_num), float(tris_across)));\n"
		"  int j = tris_across - 1 - tri_num / tris_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / tris_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 120\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_within_tri");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "tri_scale"), tri_scale);
	glUniform1f(glGetUniformLocation(prog, "rotation_delta"),
		    rotation_delta);
	glUniform1i(glGetUniformLocation(prog, "tris_across"), tris_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	tri_num_loc = glGetUniformLocation(prog, "tri_num");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_within_tri), pos_within_tri,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(pos_within_tri[0]), GL_FLOAT,
			      GL_FALSE, sizeof(pos_within_tri[0]), (void *) 0);
}

void Triangles::draw(const float (*proj)[4])
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int tri_num = 0; tri_num < num_tris; ++tri_num) {
		glUniform1i(tri_num_loc, tri_num);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}


InterpolationTestPattern::InterpolationTestPattern(const char *frag)
	: frag(frag), viewport_size_loc(0)
{
}


void
InterpolationTestPattern::compile()
{
	static struct vertex_attributes {
		float pos_within_tri[2];
		float barycentric_coords[3];
	} vertex_data[] = {
		{ { -0.5, -1.0 }, { 1, 0, 0 } },
		{ {  0.0,  1.0 }, { 0, 1, 0 } },
		{ {  0.5, -1.0 }, { 0, 0, 1 } }
	};

	/* Number of triangle instances across (and down) */
	int tris_across = 8;

	/* Total number of triangles drawn */
	num_tris = tris_across * tris_across;

	/* Scaling factor uniformly applied to triangle coords */
	float tri_scale = 0.8 / tris_across;

	/* Amount each triangle should be rotated compared to prev */
	float rotation_delta = M_PI * 2.0 / num_tris;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos_within_tri;\n"
		"attribute vec3 in_barycentric_coords;\n"
		"varying vec3 barycentric_coords;\n"
		"centroid varying vec3 barycentric_coords_centroid;\n"
		"varying vec2 pixel_pos;\n"
		"centroid varying vec2 pixel_pos_centroid;\n"
		"uniform float tri_scale;\n"
		"uniform float rotation_delta;\n"
		"uniform int tris_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int tri_num; /* [0, num_tris) */\n"
		"uniform ivec2 viewport_size;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = tri_scale * pos_within_tri;\n"
		"  float rotation = rotation_delta * tri_num;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  int i = int(mod(float(tri_num), float(tris_across)));\n"
		"  int j = tris_across - 1 - tri_num / tris_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / tris_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"  barycentric_coords = barycentric_coords_centroid =\n"
		"    in_barycentric_coords;\n"
		"  pixel_pos = pixel_pos_centroid =\n"
		"    vec2(viewport_size) * (pos + 1.0) / 2.0;\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_within_tri");
	glBindAttribLocation(prog, 1, "in_barycentric_coords");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "tri_scale"), tri_scale);
	glUniform1f(glGetUniformLocation(prog, "rotation_delta"),
		    rotation_delta);
	glUniform1i(glGetUniformLocation(prog, "tris_across"), tris_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	tri_num_loc = glGetUniformLocation(prog, "tri_num");
	viewport_size_loc = glGetUniformLocation(prog, "viewport_size");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(vertex_data[0].pos_within_tri),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) offsetof(vertex_attributes,
						pos_within_tri));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, ARRAY_SIZE(vertex_data[0].barycentric_coords),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) offsetof(vertex_attributes,
						barycentric_coords));
}


void
InterpolationTestPattern::draw(const float (*proj)[4])
{
	glUseProgram(prog);

	/* Depending what the fragment shader does, it's possible that
	 * viewport_size might get optimized away.  Only set it if it
	 * didn't.
	 */
	if (viewport_size_loc != -1) {
		GLint viewport_dims[4];
		glGetIntegerv(GL_VIEWPORT, viewport_dims);
		glUniform2i(viewport_size_loc, viewport_dims[2], viewport_dims[3]);
	}

	Triangles::draw(proj);
}


void Lines::compile()
{
	/* Line coords within (-1,-1) to (1,1) rect */
	static const float pos_line[][2] = {
		{ -0.8, -0.5 },
		{  0.8, -0.5 }
	};

	/* Number of line instances across (and down) */
	int lines_across = 4;

	/* Total number of lines drawn */
	num_lines = lines_across * lines_across;

	/* Amount each line should be rotated compared to prev */
	float rotation_delta = M_PI * 2.0 / num_lines;

	/* Scaling factor uniformly applied to line coords */
	float line_scale = 0.8 / lines_across;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos_line;\n"
		"uniform float line_scale;\n"
		"uniform float rotation_delta;\n"
		"uniform int lines_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int line_num;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = line_scale * pos_line;\n"
		"  float rotation = rotation_delta * line_num;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  int i = int(mod(float(line_num), float(lines_across)));\n"
		"  int j = lines_across - 1 - line_num / lines_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / lines_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 120\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_line");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "line_scale"), line_scale);
	glUniform1f(glGetUniformLocation(prog, "rotation_delta"),
		    rotation_delta);
	glUniform1i(glGetUniformLocation(prog, "lines_across"), lines_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	line_num_loc = glGetUniformLocation(prog, "line_num");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_line), pos_line,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(pos_line[0]), GL_FLOAT,
			      GL_FALSE, sizeof(pos_line[0]), (void *) 0);
}

void Lines::draw(const float (*proj)[4])
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int line_num = 0; line_num < num_lines; ++line_num) {
		/* Draws with line width = 0.25, 0.75, 1.25,
		 * 1.75, 2.25, 2.75, 3.25, 3.75
		 */
		glLineWidth((1 + 2 * line_num) / 4.0);
		glUniform1i(line_num_loc, line_num);
		glDrawArrays(GL_LINES, 0, 2);
	}
}

void Points::compile()
{
	/* Point coords within (-1,-1) to (1,1) rect */
	static const float pos_point[2] = { -0.5, -0.5 };

	/* Number of point instances across (and down) */
	int points_across = 4;

	/* Total number of points drawn */
	num_points = points_across * points_across;

	/* Scaling factor uniformly applied to point coords */
	float point_scale = 0.8 / points_across;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos_point;\n"
		"uniform float point_scale;\n"
		"uniform int points_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int point_num;\n"
		"uniform float depth;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = point_scale * pos_point;\n"
		"  int i = int(mod(float(point_num), float(points_across)));\n"
		"  int j = points_across - 1 - point_num / points_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / points_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, depth, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 120\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_point");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "point_scale"), point_scale);
	glUniform1i(glGetUniformLocation(prog, "points_across"), points_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	point_num_loc = glGetUniformLocation(prog, "point_num");
	depth_loc = glGetUniformLocation(prog, "depth");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_point), pos_point,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(pos_point), GL_FLOAT,
			      GL_FALSE, 0, (void *) 0);
}

void Points::draw(const float (*proj)[4])
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	glUniform1f(depth_loc, 0.0);
	for (int point_num = 0; point_num < num_points; ++point_num) {
		glPointSize((1.0 + 4 * point_num) / 4.0);
		glUniform1i(point_num_loc, point_num);
		glDrawArrays(GL_POINTS, 0, 1);
	}
}

Sunburst::Sunburst()
	: out_type(GL_UNSIGNED_NORMALIZED),
	  compute_depth(false),
	  prog(0),
	  rotation_loc(0),
	  vert_depth_loc(0),
	  frag_depth_loc(0),
	  proj_loc(0),
	  draw_colors_loc(0),
	  vao(0),
	  num_tris(0),
	  vertex_buf(0)
{
}


/**
 * Determine the GLSL type that should be used for rendering, based on
 * out_type.
 */
const char *
Sunburst::get_out_type_glsl() const
{
	switch(out_type) {
	case GL_INT:
		return "ivec4";
	case GL_UNSIGNED_INT:
		return "uvec4";
	case GL_UNSIGNED_NORMALIZED:
	case GL_FLOAT:
		return "vec4";
	default:
		printf("Unrecognized out_type: %s\n",
		       piglit_get_gl_enum_name(out_type));
		piglit_report_result(PIGLIT_FAIL);
		return "UNKNOWN";
	}
}


void Sunburst::compile()
{
	static struct vertex_attributes {
		float pos_within_tri[2];
		float barycentric_coords[3];
	} vertex_data[] = {
		{ { -0.3, -0.8 }, { 1, 0, 0 } },
		{ {  0.0,  1.0 }, { 0, 1, 0 } },
		{ {  0.3, -0.8 }, { 0, 0, 1 } }
	};
        bool need_glsl130 = out_type == GL_INT || out_type == GL_UNSIGNED_INT;

	if (need_glsl130) {
		piglit_require_gl_version(30);
	}

	/* Total number of triangles drawn */
	num_tris = 7;

	static const char *vert_template =
		"#version %s\n"
		"attribute vec2 pos_within_tri;\n"
		"attribute vec3 in_barycentric_coords;\n"
		"varying vec3 barycentric_coords;\n"
		"uniform float rotation;\n"
		"uniform float vert_depth;\n"
		"uniform mat4 proj;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = pos_within_tri;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  gl_Position = proj * vec4(pos, vert_depth, 1.0);\n"
		"  barycentric_coords = in_barycentric_coords;\n"
		"}\n";

	static const char *frag_template =
		"#version %s\n"
		"#define OUT_TYPE %s\n"
		"#define COMPUTE_DEPTH %s\n"
		"uniform float frag_depth;\n"
		"varying vec3 barycentric_coords;\n"
		"uniform mat3x4 draw_colors;\n"
		"#if __VERSION__ == 130\n"
		"  out OUT_TYPE frag_out;\n"
		"#endif\n"
		"\n"
		"void main()\n"
		"{\n"
		"#if __VERSION__ == 130\n"
		"  frag_out = OUT_TYPE(draw_colors * barycentric_coords);\n"
		"#else\n"
		"  gl_FragColor = draw_colors * barycentric_coords;\n"
		"#endif\n"
		"#if COMPUTE_DEPTH\n"
		"  gl_FragDepth = (frag_depth + 1.0) / 2.0;\n"
		"#endif\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	unsigned vert_alloc_len =
		strlen(vert_template) + 4;
	char *vert = (char *) malloc(vert_alloc_len);
	sprintf(vert, vert_template, need_glsl130 ? "130" : "120");
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	free(vert);
	glAttachShader(prog, vs);

	const char *out_type_glsl = get_out_type_glsl();
	unsigned frag_alloc_len =
		strlen(frag_template) + strlen(out_type_glsl) + 4;
	char *frag = (char *) malloc(frag_alloc_len);
	sprintf(frag, frag_template, need_glsl130 ? "130" : "120",
		out_type_glsl,
		compute_depth ? "1" : "0");
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	free(frag);
	glAttachShader(prog, fs);

	glBindAttribLocation(prog, 0, "pos_within_tri");
	glBindAttribLocation(prog, 1, "in_barycentric_coords");
	if (need_glsl130) {
		glBindFragDataLocation(prog, 0, "frag_out");
	}
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	rotation_loc = glGetUniformLocation(prog, "rotation");
	vert_depth_loc = glGetUniformLocation(prog, "vert_depth");
	frag_depth_loc = glGetUniformLocation(prog, "frag_depth");
	glUniform1f(vert_depth_loc, 0.0);
	glUniform1f(frag_depth_loc, 0.0);
	proj_loc = glGetUniformLocation(prog, "proj");
	draw_colors_loc = glGetUniformLocation(prog, "draw_colors");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(vertex_data[0].pos_within_tri),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, ARRAY_SIZE(vertex_data[0].barycentric_coords),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) offsetof(vertex_attributes,
						barycentric_coords));
}


ColorGradientSunburst::ColorGradientSunburst(GLenum out_type)
{
	this->out_type = out_type;
}


/**
 * Draw the color gradient sunburst, but instead of using color
 * components that range from 0.0 to 1.0, apply the given scaling
 * factor and offset to each color component.
 *
 * The offset is also applied when clearing the color buffer.
 */
void
ColorGradientSunburst::draw_with_scale_and_offset(const float (*proj)[4],
						  float scale, float offset)
{
	switch (out_type) {
	case GL_INT: {
		int clear_color[4] = { offset, offset, offset, offset };
		glClearBufferiv(GL_COLOR, 0, clear_color);
		break;
	}
	case GL_UNSIGNED_INT: {
		unsigned clear_color[4] = { offset, offset, offset, offset };
		glClearBufferuiv(GL_COLOR, 0, clear_color);
		break;
	}
	case GL_UNSIGNED_NORMALIZED:
	case GL_FLOAT: {
		glClearColor(offset, offset, offset, offset);
		glClear(GL_COLOR_BUFFER_BIT);
		break;
	}
	default:
		printf("Unrecognized out_type: %s\n",
		       piglit_get_gl_enum_name(out_type));
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	float draw_colors[3][4] =
		{ { 1, 0, 0, 1.0 }, { 0, 1, 0, 0.5 }, { 0, 0, 1, 1.0 } };
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 4; ++j) {
			draw_colors[i][j] = scale * draw_colors[i][j] + offset;
		}
	}
	glUniformMatrix3x4fv(draw_colors_loc, 1, GL_FALSE,
			     &draw_colors[0][0]);
	glBindVertexArray(vao);
	for (int i = 0; i < num_tris; ++i) {
		glUniform1f(rotation_loc, M_PI * 2.0 * i / num_tris);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}


void
ColorGradientSunburst::draw(const float (*proj)[4])
{
	draw_with_scale_and_offset(proj, 1.0, 0.0);
}


void
StencilSunburst::draw(const float (*proj)[4])
{
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int i = 0; i < num_tris; ++i) {
		glStencilFunc(GL_ALWAYS, i+1, 0xff);
		glUniform1f(rotation_loc, M_PI * 2.0 * i / num_tris);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	glDisable(GL_STENCIL_TEST);
}


DepthSunburst::DepthSunburst(bool compute_depth)
{
	this->compute_depth = compute_depth;
}


void
DepthSunburst::draw(const float (*proj)[4])
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int i = 0; i < num_tris; ++i) {
		/* Draw triangles in a haphazard order so we can
		 * verify that depth comparisons sort them out
		 * properly.
		 */
		int triangle_to_draw = (i * 3) % num_tris;

		/* Note: with num_tris == 7, this causes us to draw
		 * triangles at depths of 3/4, 1/2, -1/4, 0, 1/4, 1/2,
		 * and 3/4.
		 */
		glUniform1f(compute_depth ? frag_depth_loc : vert_depth_loc,
			    float(num_tris - triangle_to_draw * 2 - 1)
			    / (num_tris + 1));

		glUniform1f(rotation_loc,
			    M_PI * 2.0 * triangle_to_draw / num_tris);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	glDisable(GL_DEPTH_TEST);
}


void
ManifestStencil::compile()
{
	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 120\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = color;\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	float vertex_data[4][2] = {
		{ -1, -1 },
		{ -1,  1 },
		{  1, -1 },
		{  1,  1 }
	};
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) 0);
}

void
ManifestStencil::run()
{
	static float colors[8][4] = {
		{ 0.0, 0.0, 0.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 }
	};

	glUseProgram(prog);
	glBindVertexArray(vao);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	/* Clear the color buffer to 0, in case the stencil buffer
	 * contains any values outside the range 0..7
	 */
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < 8; ++i) {
		glStencilFunc(GL_EQUAL, i, 0xff);
		glUniform4fv(color_loc, 1, colors[i]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	}

	glDisable(GL_STENCIL_TEST);
}

void
ManifestDepth::compile()
{
	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos;\n"
		"uniform float depth;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, depth, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 120\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = color;\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");
	depth_loc = glGetUniformLocation(prog, "depth");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	float vertex_data[4][2] = {
		{ -1, -1 },
		{ -1,  1 },
		{  1, -1 },
		{  1,  1 }
	};
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) 0);
}

void
ManifestDepth::run()
{
	static float colors[8][4] = {
		{ 0.0, 0.0, 0.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 }
	};

	glUseProgram(prog);
	glBindVertexArray(vao);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glStencilFunc(GL_EQUAL, 0, 0xff);

	/* Clear the stencil buffer to 0, leaving depth and color
	 * buffers unchanged.
	 */
	glClear(GL_STENCIL_BUFFER_BIT);

	for (int i = 0; i < 8; ++i) {
		glUniform4fv(color_loc, 1, colors[i]);
		glUniform1f(depth_loc, float(7 - 2*i)/8);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	}

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
}
