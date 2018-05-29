#include "Monster.h"



Monster::Monster(GLuint _shader, const char* path)
	:
	Object(_shader)
{
	m_pModel = std::make_unique<Model>(path, _shader);
}


Monster::~Monster()
{
}

void Monster::SetPosition(glm::vec3 deltaPos)
{
	m_position = deltaPos;
}

glm::vec3 Monster::GetPosition()
{
	return m_position;
}
