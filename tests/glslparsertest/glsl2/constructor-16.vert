// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS */
#version 120
uniform vec2 u1;

void main()
{
  mat4x3 m = mat4x3(vec3(1.0,u1),
		    vec3(0.0,1.0,0.0),
		    vec3(0.0,0.0,1.0),
		    vec3(0.0,0.0,0.0));
}
