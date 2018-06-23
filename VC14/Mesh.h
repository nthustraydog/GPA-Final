#ifndef _MESH_H_
#define _MESH_H_
#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"  
#include "../Externals/Include/assimp/scene.h"      
#include "../Externals/Include/assimp/postprocess.h"
#include <string>
#include <vector>
#include <sstream>

using namespace std;
using namespace glm;

typedef struct Vertex{
	vec3 position;
	vec2 texCoords;
	vec3 normal;
	vec3 tangent;
}Vertex;

typedef struct Texture {
	GLuint id;
	aiTextureType type;
	string path;
}Texture;

class Mesh {
public:
	Mesh() : vao(0), vbo(0), ebo(0){}

	Mesh(const vector<Vertex> &vertexData, const vector<Texture> &textures, const vector<GLuint> &indices) : vao(0), vbo(0), ebo(0) {
		setData(vertexData, textures, indices);
	}
	void setData(const std::vector<Vertex>& vertData,
		const std::vector<Texture> & textures,
		const std::vector<GLuint>& indices)
	{
		this->vertexData = vertData;
		this->indices = indices;
		this->textures = textures;

		if (!vertData.empty() && !indices.empty())
		{
			this->SetUpMesh();
		}
	}
	void Move(float offsetX, float offsetY, float offsetZ) {
		for (int i = 0; i < vertexData.size(); i++) {
			vertexData[i].position.x += offsetX;
			vertexData[i].position.y += offsetY;
			vertexData[i].position.z += offsetZ;
		}
		this->UpdateMesh();
	}
	void Draw(GLuint program) const
	{
		if (vao == 0 || vbo == 0 || ebo == 0)
			return;

		glUseProgram(program);
		glBindVertexArray(vao);

		int diffuseCnt = 0, specularCnt = 0, textUnitCnt = 0;

		for (vector<Texture>::const_iterator it = textures.begin(); it != textures.end(); it++) {
			stringstream samplerNameStr;
			switch (it->type)
			{
			case aiTextureType_DIFFUSE:
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, it->id);

				samplerNameStr << "texture_diffuse" << diffuseCnt++;
				glUniform1i(glGetUniformLocation(program,
					samplerNameStr.str().c_str()), 0);

				break;
			case aiTextureType_SPECULAR:
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, it->id);

				samplerNameStr << "texture_normal" << specularCnt++;
				glUniform1i(glGetUniformLocation(program,
					samplerNameStr.str().c_str()), 1);
				
				break;
			default:
				std::cerr << "Warning::Mesh::draw, texture type" << it->type
					<< " current not supported." << std::endl;
				break;
			}
		}
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);
	}
	void final() const
	{
		glDeleteVertexArrays(1, &this->vao);
		glDeleteBuffers(1, &this->vbo);
		glDeleteBuffers(1, &this->ebo);
	}
private:
	vector<Vertex> vertexData;
	vector<GLuint> indices;
	vector<Texture> textures;
	GLuint vao, vbo, ebo;

	void SetUpMesh() {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexData.size(), &vertexData[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(5 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(8 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	void UpdateMesh() {
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexData.size(), &vertexData[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(5 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

};

#endif