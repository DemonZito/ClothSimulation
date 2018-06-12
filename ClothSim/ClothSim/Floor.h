#pragma once

#include "GlobalIncludes.h"
#include "Object.h"
#include "Model.h"

class Floor :
	public Object
{
public:
	Floor(GLuint _shader, const char* path);
	~Floor();

	void SetPosition(glm::vec3);
	glm::vec3 GetPosition();

};

