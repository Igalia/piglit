// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]

#version 150
#extension GL_ARB_tessellation_shader: require

layout(vertices = 3) out;
uniform int n;

/* If a per-vertex output variable is used as an l-value, it is an
 * error if the expression indicating the vertex number is not the
 * identifier "gl_InvocationID".
 */

void main()
{
	gl_out[n].gl_Position = vec4(0);
}
