#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include "GL/glut.h"

/*Misbehaviour: first: the quads are not drawn in the correct order (darker equals closer to the viewer), second: the middle one is strangely distorted. */
/*Author: Edwin Moehlheinrich*/
/*This piece of code is public domain, just don't get yourself killed.*/

void main(int argc, char** argv)
{

	float matrix[] = {	2.0/2.0, 0.0, 0.0, 0.0,
				0.0, 2.0/2.0, 0.0, 0.0,
				0.0, 0.0, -2/10.0, -12.0/10,//far = 11, near = 1
				0.0, 0.0, 0.0, 1.0};
		
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); 
	
	SDL_Surface *screen = SDL_SetVideoMode(600, 600, 24, SDL_OPENGL);


	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	GLuint _vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	char vertexShader[] = 	"uniform mat4 eye_projection;"
				"varying vec2 texture_coords;"
				"void main(){"
				"gl_Position = eye_projection * gl_Vertex;"
				"texture_coords = gl_MultiTexCoord0.st;}";
	char *vertexShaderPtr = vertexShader;
	
	glShaderSourceARB(_vertexShader, 1, &vertexShaderPtr,NULL);
	glCompileShaderARB(_vertexShader);
	
	GLuint _fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	char fragmentShader[] = "uniform sampler2D shadowMap;"
				"varying vec2 texture_coords;"
				"void main(){"
				"float map_depth = texture2D(shadowMap,texture_coords).a;"
				"gl_FragColor =  vec4(1.0, 1.0, 1.0, 1.0)* map_depth;}";
	char *fragmentShaderPtr = fragmentShader;
	glShaderSourceARB(_fragmentShader, 1, &fragmentShaderPtr,NULL);
	glCompileShaderARB(_fragmentShader);
	
	GLuint _shaderProgram = glCreateProgramObjectARB();
	glAttachObjectARB(_shaderProgram,_vertexShader);
	glAttachObjectARB(_shaderProgram,_fragmentShader);
	glLinkProgramARB(_shaderProgram);	

	GLuint _vertexShaderShadow = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	char vertexShader2[] = 	"uniform mat4 light_projection; "
				"void main(){"
				"gl_Position = light_projection * gl_Vertex;}";
	vertexShaderPtr = vertexShader2;
	glShaderSourceARB(_vertexShaderShadow, 1, &vertexShaderPtr,NULL);
	glCompileShaderARB(_vertexShaderShadow);
	
	GLuint _fragmentShaderShadow = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	char fragmentShader2[] = "void main(){gl_FragDepth = gl_FragCoord.z;}";
	fragmentShaderPtr = fragmentShader2;
	glShaderSourceARB(_fragmentShaderShadow, 1, &fragmentShaderPtr,NULL);
	glCompileShaderARB(_fragmentShaderShadow);

	GLuint _shaderProgramShadow = glCreateProgramObjectARB();
	glAttachObjectARB(_shaderProgramShadow,_vertexShaderShadow);
	glAttachObjectARB(_shaderProgramShadow,_fragmentShaderShadow);
	glLinkProgramARB(_shaderProgramShadow);	
	
	GLuint eye_projection = glGetUniformLocationARB(_shaderProgram, "eye_projection");
	GLuint light_projection = glGetUniformLocationARB(_shaderProgramShadow, "light_projection");

	glEnable(GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST);
	glClearDepth(1.0f);
    	glDepthFunc(GL_LEQUAL);
	
	GLuint _shadowMap;
	GLuint shadertexreference = glGetUniformLocationARB(_shaderProgram, "shadowMap");
	glUniform1iARB(shadertexreference, 0);
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &_shadowMap);
	glBindTexture(GL_TEXTURE_2D, _shadowMap);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);
	
	GLuint _frameBufferObject;
	glGenFramebuffersEXT(1, &_frameBufferObject);	

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _frameBufferObject);
	glDrawBuffer(GL_NONE); 
	glReadBuffer(GL_NONE); 
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, _shadowMap, 0); 
	
	glViewport(0,0,512, 512);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgramObjectARB(_shaderProgramShadow);
	glUniformMatrix4fvARB(light_projection,1, 1, matrix);//ogl reads in col-vectors,so transform=true
	glBegin(GL_QUADS);
		glVertex3f(-0.4,   0.4,   -2.0);
		glVertex3f(-0.4,  -0.4,   -2.0);
		glVertex3f(0.4,  -0.4,   -2.0);
		glVertex3f(0.4,   0.4,   -2.0);	
		
		glVertex3f(-0.2,   0.5,   -7.0);
		glVertex3f(-0.2,  -0.3,   -1.0);
		glVertex3f(0.6,  -0.3,   -1.0);
		glVertex3f(0.6,   0.5,   -7.0);
	
		glVertex3f(-0.0,   0.6,   -4.0);
		glVertex3f(-0.0,  -0.2,   -4.0);
		glVertex3f(0.8,  -0.2,   -4.0);
		glVertex3f(0.8,   0.6,   -4.0);	
		
	glEnd();	
		
	//bind back the backbuffer and display the depthmap
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(0,0,600, 600);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, _shadowMap);
	
	glUseProgramObjectARB(_shaderProgram);
	glUniformMatrix4fvARB(eye_projection,1, 1, matrix);
	glBegin(GL_QUADS);//this is the quad that is texturized with what we want to see
	
		glTexCoord2d(0.0,1.0);
		glVertex3f(-0.9,   0.9,   -1);
		
		glTexCoord2d(0.0,0.0);
		glVertex3f(-0.9,  -0.9,   -1);

		glTexCoord2d(1.0,0.0);
		glVertex3f(0.9,  -0.9,   -1);
		
		glTexCoord2d(1.0,1.0);
		glVertex3f(0.9,   0.9,   -1);	

	glEnd();
	SDL_GL_SwapBuffers();
	SDL_Delay(3000);//just for visualising the effect
}
