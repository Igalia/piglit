# test emitting a ${type_name} from vs->fs works
# when originally written this failed in varying lowering

[require]
GLSL >= ${glsl_version}
% if glsl_version == '1.50':
GL_ARB_gpu_shader_fp64
% endif

[vertex shader]
#version ${glsl_version_int}
% if glsl_version == '1.50':
#extension GL_ARB_gpu_shader_fp64 : require
% endif

uniform double arg0;

in vec4 vertex;
flat out ${type_name} dout1;

void main()
{
	gl_Position = vertex;
	dout1 = ${type_name}(arg0);
}

[fragment shader]
#version ${glsl_version_int}
% if glsl_version == '1.50':
#extension GL_ARB_gpu_shader_fp64 : require
% endif

flat in ${type_name} dout1;
uniform double tolerance;
uniform double expected;
out vec4 color;

void main()
{
	${type_name} result = trunc(dout1);
	color = distance(result, ${type_name}(expected)) <= tolerance
		? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}

[vertex data]
vertex/float/2
-1.0 -1.0
 1.0 -1.0
 1.0  1.0
-1.0  1.0

[test]
clear color 0.0 0.0 0.0 0.0

clear
uniform double arg0 1.7976931348623157E+308
uniform double expected 1.7976931348623157E+308
uniform double tolerance 2.0000000000000002e-05
draw arrays GL_TRIANGLE_FAN 0 4
probe rgba 0 0 0.0 1.0 0.0 1.0
