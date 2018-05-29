#pragma once

#include "GlobalIncludes.h"

class UI
{
public:
	UI(glm::vec2 _position, glm::vec2 _scale, GLuint _shader);
	~UI();

	virtual void Render() const = 0;

protected:
	glm::vec2 m_position;
	glm::vec2 m_scale;

	GLuint m_shader;

	GLuint VBO, VAO, EBO;

};

