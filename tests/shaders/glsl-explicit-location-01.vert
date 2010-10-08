#extension GL_ARB_explicit_attrib_location: require

#ifdef GL_ARB_explicit_attrib_location
layout(location = 0) attribute vec4 vertex;

void main()
{
	gl_Position = vertex;
}
#endif
