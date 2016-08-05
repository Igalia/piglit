/* [config]
 * expect_result: fail
 * glsl_version: 1.30
 * require_extensions: GL_ARB_shading_language_420pack
 * [end config]
 */

#version 130
#extension GL_ARB_shading_language_420pack: enable

struct s { float f; };

void main() {
    // Illegal since e.b is struct s, but bool given as initializer
    struct {
        float a;
        s b;
    } e = { 1.2, true };
    gl_FragColor = vec4(1.0);
}
