#extension GL_ARB_explicit_attrib_location: require

layout(location = 1000) attribute vec4 vertex;

void main()
{
	gl_Position = vertex;
}
