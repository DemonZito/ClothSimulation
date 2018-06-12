#pragma once

#include "GlobalIncludes.h"
#include "Object.h"

class Skybox : public Object
{
public:
	Skybox(std::vector<std::string> _strImagePaths, GLuint _shader);
	~Skybox();

	void CreateEBO();

	void Render() override;

private:
	// Gets the location of some variables for the shader for later edditiong
	void GetUniformLocation();
	void SetCubeVertices();

	void SetCubeIndices();

	// Creates the texture and sends it to the shader
	void CreateAndBindTexture();

	void CreateVBOAttributes();

	GLuint m_iCubeSampler, m_iTextureLocation;
	GLuint VBO, VAO, EBO;


	Texture* pTexture;
	std::vector<std::string> m_strImagePaths;
	Vertex* m_vertices;
	GLuint* m_indices;
};
