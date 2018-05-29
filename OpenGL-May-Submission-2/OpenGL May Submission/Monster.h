#pragma once

#include "GlobalIncludes.h"
#include "Object.h"
#include "Model.h"

class Monster :
	public Object
{
public:
	Monster(GLuint _shader, const char* path);
	~Monster();

	void SetPosition(glm::vec3);
	glm::vec3 GetPosition();

};

