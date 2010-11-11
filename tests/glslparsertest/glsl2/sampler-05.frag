// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL
 *
 * From page 17 (page 23 of the PDF) of the GLSL 1.20 spec:
 *
 *    "Samplers cannot be treated as l-values; hence cannot be used as
 *     out or inout function parameters, nor can they be assigned into."
 */
uniform sampler2D u[4];
varying vec2 coord;

void function(out sampler2D s, int i)
{
  s = u[i];
}

void main()
{
  sampler2D temp;
  function(temp, 0);
  gl_FragColor = texture2D(temp, coord);
}
