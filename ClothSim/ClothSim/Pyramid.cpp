#include "Pyramid.h"



Pyramid::Pyramid(GLuint _shader)
	:
	Object(_shader)
{
	m_pModel = std::make_unique<Model>("Resources/Models/Pyramid.obj", _shader);
}


Pyramid::~Pyramid()
{
}

void Pyramid::SetPosition(glm::vec3 _pos)
{
	m_position = _pos;
}

glm::vec3 Pyramid::GetPosition()
{
	return m_position;
}

void Pyramid::SetScale(glm::vec3 _scale)
{
	m_scale = _scale;
}

glm::vec3 Pyramid::GetScale()
{
	return m_scale;
}
