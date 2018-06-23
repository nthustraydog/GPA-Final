#pragma once

#ifdef _MSC_VER
#include "GLEW/glew.h"
#include "FreeGLUT/freeglut.h"
#include <direct.h>
#else
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#include <unistd.h>
#endif

void shaderLog(GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete[] errorLog;
	}
}
