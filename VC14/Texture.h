#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"  
#include "../Externals/Include/assimp/scene.h"      
#include "../Externals/Include/assimp/postprocess.h"
#include <iostream>
#include <fstream>

typedef struct _TextureData
{
	_TextureData(void) :
		width(0),
		height(0),
		data(0)
	{
	}

	int width;
	int height;
	unsigned char* data;
} TextureData;

class TextureHelper
{
public:
	
	static  GLuint load2DTexture(const char* filename, GLint internalFormat = GL_RGBA8,
		GLenum picFormat = GL_RGBA)
	{
		
		GLuint textureId = 0;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
									
		TextureData imageTexData;
		GLubyte *imageData = NULL;
		int picWidth, picHeight;
		int channels = 0;

		imageTexData = loadPNG(filename);
		imageData = imageTexData.data;

		if (imageData == NULL)
		{
			std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
			return 0;
		}
		//printf("Loaded image with width[%d], height[%d]\n", imageTexData.width, imageTexData.height);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageTexData.width, imageTexData.height,
			0, picFormat, GL_UNSIGNED_BYTE, imageData);

		
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		return textureId;
	}
#define FOURCC_DXT1 0x31545844 
#define FOURCC_DXT3 0x33545844 
#define FOURCC_DXT5 0x35545844 

	static GLuint loadDDS(const char * filename) {


		/* try to open the file */
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		if (!file) {
			std::cout << "Error::loadDDs, could not open:"
				<< filename << "for read." << std::endl;
			return 0;
		}

		/* verify the type of file */
		char filecode[4];
		file.read(filecode, 4);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			std::cout << "Error::loadDDs, format is not dds :"
				<< filename << std::endl;
			file.close();
			return 0;
		}

		/* get the surface desc */
		char header[124];
		file.read(header, 124);

		unsigned int height = *(unsigned int*)&(header[8]);
		unsigned int width = *(unsigned int*)&(header[12]);
		unsigned int linearSize = *(unsigned int*)&(header[16]);
		unsigned int mipMapCount = *(unsigned int*)&(header[24]);
		unsigned int fourCC = *(unsigned int*)&(header[80]);


		char * buffer = NULL;
		unsigned int bufsize;
		/* how big is it going to be including all mipmaps? */
		bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
		buffer = new char[bufsize];
		file.read(buffer, bufsize);
		/* close the file pointer */
		file.close();

		unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
		unsigned int format;
		switch (fourCC)
		{
		case FOURCC_DXT1:
			format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case FOURCC_DXT3:
			format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case FOURCC_DXT5:
			format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			delete[] buffer;
			return 0;
		}

		// Create one OpenGL texture
		GLuint textureID;
		glGenTextures(1, &textureID);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
		unsigned int offset = 0;

		/* load the mipmaps */
		for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
		{
			unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
				0, size, buffer + offset);

			offset += size;
			width /= 2;
			height /= 2;

			// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
			if (width < 1) width = 1;
			if (height < 1) height = 1;

		}

		delete[] buffer;

		return textureID;
	}

	static TextureData loadPNG(const char* const pngFilepath)
	{
		TextureData texture;
		int components;

		// load the texture with stb image, force RGBA (4 components required)
		stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);

		// is the image successfully loaded?
		if (data != NULL)
		{
			// copy the raw data
			size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
			texture.data = new unsigned char[dataSize];
			memcpy(texture.data, data, dataSize);

			// mirror the image vertically to comply with OpenGL convention
			for (size_t i = 0; i < texture.width; ++i)
			{
				for (size_t j = 0; j < texture.height / 2; ++j)
				{
					for (size_t k = 0; k < 4; ++k)
					{
						size_t coord1 = (j * texture.width + i) * 4 + k;
						size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
						std::swap(texture.data[coord1], texture.data[coord2]);
					}
				}
			}

			// release the loaded image
			stbi_image_free(data);
		}

		return texture;
	}
};

#endif