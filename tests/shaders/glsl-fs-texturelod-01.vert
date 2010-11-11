/**
 * \file glsl-fs-texture-lod-01.vert
 */

#version 120

varying vec2 texcoord;

void main()
{
	texcoord = (vec2(gl_Vertex) + 1.0) / 2.0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
