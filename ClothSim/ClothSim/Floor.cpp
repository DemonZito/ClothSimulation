#include "Floor.h"



Floor::Floor(GLuint _shader, const char* path)
	:
	Object(_shader)
{
	m_pModel = std::make_unique<Model>(path, _shader);
}


Floor::~Floor()
{
}

void Floor::SetPosition(glm::vec3 deltaPos)
{
	m_position = deltaPos;
}

glm::vec3 Floor::GetPosition()
{
	return m_position;
}
