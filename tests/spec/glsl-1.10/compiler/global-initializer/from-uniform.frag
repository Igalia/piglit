/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * [end config]
 */

uniform float uf;
float gf1 = 1.0;
float gf2 = uf;

void main()
{
    gl_FragColor = vec4(gf1, gf2, uf, 1.0);
}
