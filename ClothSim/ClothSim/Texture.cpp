#include "texture.h"

Texture::Texture(GLenum _textureTarget, std::string _textureFileName) {
	textureTarget = _textureTarget;
	textureFileName = _textureFileName;
}

Texture::Texture(GLenum _textureTarget, std::vector<std::string> _textureFileNames)
{
	textureTarget = _textureTarget;
	textureFileNames = _textureFileNames;
}

bool Texture::load() {
	image = SOIL_load_image(
		textureFileName.c_str(),  // file name
		&imageWidth,              // width of the image
		&imageHeight,             // height of the image
		0,                        // number of channels
		SOIL_LOAD_RGBA);

	if (image != NULL) {
		glGenTextures(1, &textureObject);
		glBindTexture(textureTarget, textureObject);
		glTexImage2D(textureTarget, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);


		glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(textureTarget, 0);
		return true;
	}

	return false;
}

bool Texture::loadCubemap()
{
	glGenTextures(1, &textureObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < textureFileNames.size(); i++)
	{
		unsigned char *data = SOIL_load_image(textureFileNames[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			SOIL_free_image_data(data);
		}
		else
		{

		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return true;
}

void Texture::bind(GLenum textureUnit) {
	glActiveTexture(textureUnit);
	glBindTexture(textureTarget, textureObject);
}