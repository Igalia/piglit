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
 * Based on the Regnum Online shader in bugzilla #28138.
 * */
#version 120

#line 336
#extension GL_ARB_texture_rectangle : enable
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)

varying vec4 clrAdd;
varying vec4 clrAddSmooth;
uniform float g_fBloomAddSmoothFactor;

void main()
{
  gl_FragColor.xyz = mix(clrAdd, clrAddSmooth, g_fBloomAddSmoothFactor).xyz;
  gl_FragColor.w = 1.0;
}
