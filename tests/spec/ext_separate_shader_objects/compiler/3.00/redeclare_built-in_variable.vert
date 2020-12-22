// [config]
// expect_result: pass
// glsl_version: 3.00
// require_extensions: EXT_separate_shader_objects
// [end config]
//
// From the EXT_separate_shader_objects:
//
// "The following vertex shader outputs may be redeclared
//  at global scope to specify a built-in output interface,
//  with or without special qualifiers:
//
//  gl_Position
//  gl_PointSize
//
//  When compiling shaders using either of the above variables,
//  both such variables must be redeclared prior to use."

#version 300 es
#extension GL_EXT_separate_shader_objects : require
layout (location = 0) in vec3 aPos;
out highp vec4 gl_Position;
out highp float gl_PointSize;
out vec4 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    gl_PointSize = gl_Position.x;
    vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
}
