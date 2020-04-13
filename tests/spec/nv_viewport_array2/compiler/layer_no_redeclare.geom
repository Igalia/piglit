// [config]
// expect_result: fail
// glsl_version: 1.50
// check_link: false
// [end config]

#version 150

// Redeclaration is not allowed in unextended GL
out int gl_Layer;

void main()
{
  gl_Layer = 1;
}
