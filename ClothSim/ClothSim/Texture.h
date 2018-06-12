#ifndef _TEXTURE_H__
#define _TEXTURE_H__

#include <glad\glad.h>
#include <soil\SOIL.h>
#include <string>
#include <vector>
#include <iostream>


class Texture {
private:
	std::string textureFileName;
	std::vector<std::string> textureFileNames;
	GLenum textureTarget;
	GLuint textureObject;
	unsigned char* image;
	int imageWidth;
	int imageHeight;
public:
	Texture(GLenum TextureTarget, std::string textureFileName);
	Texture(GLenum TextureTarget, std::vector<std::string> textureFileNames);
	bool load();
	bool loadCubemap();
	void bind(GLenum textureUnit);
};

#endif //_TEXTURE_H__