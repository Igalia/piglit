/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * [end config]
 */

varying float af;
float gf1 = 1.0;
float gf2 = af;

void main()
{
    gl_FragColor = vec4(gf1, gf2, af, 1.0);
}
