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
