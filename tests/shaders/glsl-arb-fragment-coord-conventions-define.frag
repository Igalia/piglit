void main()
{
	if (GL_ARB_fragment_coord_conventions == 1) {
		gl_FragColor = {0.0, 1.0, 0.0, 0.0};
	} else {
		gl_FragColor = {1.0, 0.0, 0.0, 0.0};
	}
}
