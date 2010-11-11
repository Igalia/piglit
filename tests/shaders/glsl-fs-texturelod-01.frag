/**
 * \file glsl-fs-texture-lod-01.frag
 */

#version 120
#extension GL_ARB_shader_texture_lod: enable

uniform sampler2D sampler;
uniform float lod;

varying vec2 texcoord;

void main()
{
	gl_FragColor = texture2DLod(sampler, texcoord, lod);
}
