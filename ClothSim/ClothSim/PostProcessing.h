#pragma once

#include "GlobalIncludes.h"

class PostProcessing
{
public:
	PostProcessing(GLuint _shader);
	~PostProcessing();
	void Initialize();

	GLuint GetFramebuffer();

	void Draw();

private:
	GLuint VAO, VBO, m_shader, textureColorbuffer, rbo, framebuffer;
};

