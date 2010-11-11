// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

uniform sampler2DRect samp;

void main()
{
	gl_FragColor = texture2DRect(samp, vec2(0.0, 0.0));
}
