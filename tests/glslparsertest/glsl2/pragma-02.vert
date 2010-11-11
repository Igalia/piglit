// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS
 *
 * From page 11 (page 17 of the PDF) of the GLSL 1.10 spec:
 *
 *     "If an implementation does not recognize the tokens following #pragma,
 *     then it will ignore that pragma."
 */
#pragma unlikely_to_be_recognized

void main()
{
  gl_Position = gl_Vertex;
}
