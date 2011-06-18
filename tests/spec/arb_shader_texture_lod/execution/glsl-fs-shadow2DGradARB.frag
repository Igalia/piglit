#extension GL_ARB_shader_texture_lod : require

uniform sampler2DShadow tex;
varying vec4 texcoord;

void main()
{
	gl_FragColor = shadow2DGradARB(tex, texcoord.xyy,
				       dFdx(texcoord.xy), dFdy(texcoord.xy));
}
