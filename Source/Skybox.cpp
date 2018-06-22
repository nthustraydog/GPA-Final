#include "../VC14/Skybox.h"
#include "../Externals/Include/STB/stb_image.h"
#include "../VC14/Camera.h"

#include <fstream>

void compileShader(GLuint shader, const char* fileName);

GLuint Skybox::program = 0;

float Skybox::vertices[12] = { -1.0f, 1.0f, 1.0f,
-1.0f, -1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, -1.0f, 1.0f };
unsigned int Skybox::indices[6] = { 0, 1, 2,
2, 1, 3 };

Skybox::Skybox(const std::vector<std::string>& skyboxPaths, glm::vec3& eyePos, glm::mat4& viewing, glm::mat4& projection) :
	viewing(viewing), projection(projection), eyePos(eyePos), vao(0), texture(0)
{
	// Load Program
	if (program == 0)
	{
		GLuint vertextShader = glCreateShader(GL_VERTEX_SHADER);
		compileShader(vertextShader, "skybox.vs.glsl");

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(fragmentShader, "skybox.fs.glsl");

		program = glCreateProgram();
		glAttachShader(program, vertextShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		glDeleteShader(vertextShader);
		glDeleteShader(fragmentShader);
	}
	// vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Vertex attribute
	GLuint posBuffer;
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// Index buffer
	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	for (unsigned i = 0; i < 6; i++)
	{
		int width, height, componentPerPixel;
		unsigned char *data = stbi_load(skyboxPaths[i].c_str(), &width, &height, &componentPerPixel, 0);

		if (data != NULL)
		{
			GLenum internalFormat, dataFormat;
			if (componentPerPixel == 3)
			{
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				internalFormat = GL_RGB8;
				dataFormat = GL_RGB;
			}
			else if (componentPerPixel == 4)
			{
				internalFormat = GL_RGBA8;
				dataFormat = GL_RGBA;
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	// Done with vao
	glBindVertexArray(0);
}

Skybox::~Skybox()
{
}
#include <iostream>
void Skybox::draw()
{
	glBindVertexArray(vao);
	glUseProgram(program);

	glm::mat4 vp = projection * viewing;

	glUniformMatrix4fv(glGetUniformLocation(program, "vpMatrix"), 1, GL_FALSE, reinterpret_cast<GLfloat*>(&vp));
	glUniform3f(glGetUniformLocation(program, "eyePos"), eyePos.x, eyePos.y, eyePos.z);

	glUniform1i(glGetUniformLocation(program, "skyboxTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	glDepthFunc(GL_LEQUAL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDepthFunc(GL_LESS);
	glBindVertexArray(0);
}

void compileShader(GLuint shader, const char* fileName)
{
	std::ifstream ifs(fileName, std::ios_base::in | std::ios_base::binary);
	if (ifs)
	{
		ifs.seekg(0, ifs.end);
		int length = static_cast<int>(ifs.tellg());
		ifs.seekg(0, ifs.beg);

		char* shaderSourceText = new char[length];
		ifs.read(shaderSourceText, length);

		glShaderSource(shader, 1, &shaderSourceText, &length);
		glCompileShader(shader);
		delete[] shaderSourceText;
	}
	ifs.close();
}
