#pragma once

#include "GlobalIncludes.h"
#include "Object.h"
#include "Model.h"

class Capsule :
	public Object
{
public:
	Capsule(GLuint _shader);
	~Capsule();

	void SetPosition(glm::vec3 pos);
	glm::vec3 GetPosition();

	void SetScale(glm::vec3 scale);

	glm::vec3 GetScale();

};