// [config]
// expect_result: fail
// glsl_version: 1.10
// require_extensions: GL_ARB_shading_language_include
// [end config]
//
// GLSL doesn't have string literals, GL_ARB_shading_language_include
// does add notion of path but it only has meaning with #line and #include

#extension GL_ARB_shading_language_include: enable

void main() {
	int i = "1";
}
