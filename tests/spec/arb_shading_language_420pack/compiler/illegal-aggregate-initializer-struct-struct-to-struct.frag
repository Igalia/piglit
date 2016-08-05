/* [config]
 * expect_result: fail
 * glsl_version: 1.30
 * require_extensions: GL_ARB_shading_language_420pack
 * [end config]
 */

#version 130
#extension GL_ARB_shading_language_420pack: enable

struct s1 { float f; };
struct s2 { bool f; bool g; };

void main() {
    // Illegal since e.b is struct s1, but a struct s2 given as initializer
    struct {
        float a;
        s1 b;
    } e = { 1.2, s2(true, false) };
    gl_FragColor = vec4(1.0);
}
