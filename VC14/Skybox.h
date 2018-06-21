#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include <vector>
#include <string>
#include "../Externals/Include/GLEW/glew.h"
#include "../Externals/Include/GLM/glm/mat4x4.hpp"
#include "../Externals/Include/GLM/glm/vec3.hpp"


class Skybox
{
public:
	Skybox(const std::vector<std::string>& skyboxPaths, glm::vec3& eyePos, glm::mat4& viewing, glm::mat4& projection);
	~Skybox();

	void draw();

protected:
	GLuint vao;
	GLuint texture;

	glm::mat4 &viewing, &projection;
	glm::vec3& eyePos;

	static GLuint program;
	static float vertices[12];
	static unsigned int indices[6];
};

#endif