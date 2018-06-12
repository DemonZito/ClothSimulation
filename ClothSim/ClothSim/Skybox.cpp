#include "Skybox.h"


Skybox::Skybox(std::vector<std::string> _strImagePaths, GLuint _shader)
	: 
	m_strImagePaths(_strImagePaths),
	Object(_shader)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	SetCubeVertices();
	SetCubeIndices();

	CreateVBOAttributes();
	CreateEBO();

	CreateAndBindTexture();

	GetUniformLocation();
}

void Skybox::SetCubeVertices()
{
	m_vertices = new Vertex[24]
	{
		{ { -1.0f,  1.0f, 1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.0f } },
		{ { 1.0f,  1.0f, 1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.5f } },
		{ { 1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 0.5f } },

		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.0f, 0.5f } },
		{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.5f } },
		{ { -1.0f,  1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },
		{ { 1.0f,  1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 1.0f } },

		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.0f } },
		{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.5f } },
		{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 0.5f } },

		{ { -1.0f,  -1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.0f, 0.5f } },
		{ { 1.0f,  -1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.5f } },
		{ { -1.0f,  -1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
		{ { 1.0f,  -1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.0f } },

		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.5f } },
		{ { -1.0f,  1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 0.5f } },
		{ { -1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 1.0f } },
		{ { -1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },

		{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 0.5f } },
		{ { 1.0f,  1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 0.5f } },
		{ { 1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f },{ 0.5f, 1.0f } },
		{ { 1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
	};
}

void Skybox::SetCubeIndices()
{
	m_indices = new GLuint[36]
	{
		0,2,1,
		1,2,3,

		4,6,5,
		5,6,7,

		8,10,9,
		9,10,11,

		12,14,13,
		13,14,15,

		16,18,17,
		17,18,19,

		20,22,21,
		21,22,23 
	};
}

Skybox::~Skybox()
{
	
}

void Skybox::GetUniformLocation()
{
	m_iCubeSampler = glGetUniformLocation(m_shader, "cubeSampler");
	assert(m_iCubeSampler != 0xFFFFFFFF);
}

void Skybox::CreateAndBindTexture()
{
	m_iTextureLocation = GL_TEXTURE_CUBE_MAP;
	pTexture = new Texture(m_iTextureLocation, m_strImagePaths);
	if (!pTexture->loadCubemap())
	{
		std::cout << "Error Loading Texture";
	}
	glUniform1i(glGetUniformLocation(m_shader, "Texture"), 0);
}

void Skybox::CreateVBOAttributes()
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (24 * 8) * sizeof(GLfloat), m_vertices, GL_STATIC_DRAW);

	GLuint aPositionLocation = glGetAttribLocation(m_shader, "a_position");
	glVertexAttribPointer(aPositionLocation, 3, GL_FLOAT, GL_FALSE,  8 * sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(aPositionLocation);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Skybox::CreateEBO()
{
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 36, m_indices, GL_STATIC_DRAW);
}


void Skybox::Render()
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	glUseProgram(m_shader);

	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	glUniformMatrix4fv(m_iScaleLocation, 1, GL_FALSE, glm::value_ptr(Scale));
	glUniformMatrix4fv(m_iRotateLocation, 1, GL_FALSE, glm::value_ptr(Rotate));
	glUniformMatrix4fv(m_iTranslateLocation, 1, GL_FALSE, glm::value_ptr(Translate));

	glUniform1i(m_iCubeSampler, 0);
	pTexture->bind(GL_TEXTURE0);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);

}
