#ifndef writeimg_h
#define writeimg_h

void WritePNGImage(const char* filename,
		GLenum format, int width, int height, GLubyte* data, int reverse);

#endif /* writeimg_h */
