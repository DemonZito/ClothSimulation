#include "Capsule.h"



Capsule::Capsule(GLuint _shader)
	:
	Object(_shader)
{
	// Load capsule model
	m_pModel = std::make_unique<Model>("Resources/Models/Bullet.obj", _shader);
}


Capsule::~Capsule()
{
}

void Capsule::SetPosition(glm::vec3 _pos)
{
	m_position = _pos;
}

glm::vec3 Capsule::GetPosition()
{
	return m_position;
}

void Capsule::SetScale(glm::vec3 _scale)
{
	m_scale = _scale;
}

glm::vec3 Capsule::GetScale()
{
	return m_scale;
}
