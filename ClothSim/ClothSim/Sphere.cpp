#include "Sphere.h"



Sphere::Sphere(GLuint _shader)
	:
	Object(_shader)
{
	m_pModel = std::make_unique<Model>("Resources/Models/Sphere.obj", _shader);
}


Sphere::~Sphere()
{
}

void Sphere::SetPosition(glm::vec3 _pos)
{
	m_position = _pos;
}

glm::vec3 Sphere::GetPosition()
{
	return m_position;
}

void Sphere::SetScale(glm::vec3 _scale)
{
	m_scale = _scale;
}

glm::vec3 Sphere::GetScale()
{
	return m_scale;
}
