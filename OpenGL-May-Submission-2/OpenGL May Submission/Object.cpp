#include "Object.h"



Object::Object(GLuint _shader)
	:
	m_shader(_shader),
	pTexture(nullptr)
{
	m_position = glm::vec3(0, 0, 0);
	GetUniformLocation();
}


Object::~Object()
{
}

void Object::Update()
{
}

void Object::Render()
{
	glUseProgram(m_shader);

	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_scale));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 Translate = glm::translate(glm::mat4(1.0f), glm::vec3(m_position));

	glUniformMatrix4fv(m_iScaleLocation, 1, GL_FALSE, glm::value_ptr(Scale));
	glUniformMatrix4fv(m_iRotateLocation, 1, GL_FALSE, glm::value_ptr(Rotate));
	glUniformMatrix4fv(m_iTranslateLocation, 1, GL_FALSE, glm::value_ptr(Translate));

	m_pModel->Draw();
}

void Object::GetUniformLocation()
{
	m_iScaleLocation = glGetUniformLocation(m_shader, "gScale");
	assert(m_iScaleLocation != 0xFFFFFFFF);

	m_iRotateLocation = glGetUniformLocation(m_shader, "gRotate");
	assert(m_iRotateLocation != 0xFFFFFFFF);

	m_iTranslateLocation = glGetUniformLocation(m_shader, "gTranslate");
	assert(m_iTranslateLocation != 0xFFFFFFFF);

	//m_icurrentTimeLocation = glGetUniformLocation(m_shader, "currentTime");
	//assert(m_icurrentTimeLocation != 0xFFFFFFFF);
}
