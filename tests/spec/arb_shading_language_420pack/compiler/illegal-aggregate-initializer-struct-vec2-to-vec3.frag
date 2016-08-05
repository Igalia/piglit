/* [config]
 * expect_result: fail
 * glsl_version: 1.30
 * require_extensions: GL_ARB_shading_language_420pack
 * [end config]
 */

#version 130
#extension GL_ARB_shading_language_420pack: enable

void main() {
    // Illegal since e.b is vec2, but a vec3 given as initializer and only “Implicit Conversions.” are allowed
    struct {
        float a;
        vec2 b;
    } e = { 1.2, vec3(1.0, 0.0, 1.0) };
    gl_FragColor = vec4(1.0);
}
