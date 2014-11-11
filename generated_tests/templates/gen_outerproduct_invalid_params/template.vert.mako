/* [config]
 * expect_result: fail
 * glsl_version: 1.20
 * [end config]
 */
#version 120
void main() {
    gl_Position = vec4(0);
    outerProduct(${type}(0), ${type}(0));
}
