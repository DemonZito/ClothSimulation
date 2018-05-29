#include "Sprite.h"



Sprite::Sprite(std::string _image, glm::vec2 _position, glm::vec2 _scale, glm::vec3 _color, GLuint _shader)
	:
	m_color(_color),
	UI(_position, _scale, _shader)
{
	GLInit();
	CreateIndicies();
	GetUniforms();
	LoadTexture(_image);
}

void Sprite::LoadTexture(std::string _image)
{
	m_texture = new Texture(GL_TEXTURE_2D, _image);

	if (!m_texture->load())
	{
		std::cout << "error loading texture" << std::endl;
	}
}


Sprite::~Sprite()
{
}

void Sprite::CreateIndicies()
{
	for (int i = 0; i < 6; i++)
	{
		m_vecIndicies.push_back(i);
	}
}

void Sprite::GLInit()
{
	// Configure VAO/VBO
	GLuint VBO;
	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Sprite::GetUniforms()
{
	m_iModel = glGetUniformLocation(m_shader, "model");
	assert(m_iModel != 0xFFFFFFFF);

	m_iSampler = glGetUniformLocation(m_shader, "image");
	assert(m_iSampler != 0xFFFFFFFF);

	m_iProjLocation = glGetUniformLocation(m_shader, "projection");
	assert(m_iProjLocation != 0xFFFFFFFF);
}

void Sprite::Render() const
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(m_shader);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH);

	// Transparency elimination
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glm::mat4 model;
	model = glm::translate(model, glm::vec3(m_position, 0.0f));

	model = glm::translate(model, glm::vec3(0.5f * m_scale.x, 0.5f * m_scale.y, 0.0f));
	model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * m_scale.x, -0.5f * m_scale.y, 0.0f));

	model = glm::scale(model, glm::vec3(m_scale, 1.0f));

	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WINDOW_WIDTH), static_cast<GLfloat>(WINDOW_HEIGHT), 0.0f, -1.0f, 1.0f);

	glUniformMatrix4fv(m_iModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(m_iProjLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glUniform1i(m_iSampler, 0);
	m_texture->bind(GL_TEXTURE0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH);

	glEnable(GL_DEPTH_TEST);
}
