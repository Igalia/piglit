/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * [end config]
 */
uniform ivec4 v;
uniform mat4 m;

void main()
{
    gl_FragColor = m[v[0]];
}
