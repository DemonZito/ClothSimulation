#pragma once

#include "GlobalIncludes.h"
#include "Object.h"
#include "Model.h"

class Sphere :
	public Object
{
public:
	Sphere(GLuint _shader, const char* path);
	~Sphere();

	void SetPosition(glm::vec3 pos);
	glm::vec3 GetPosition();

	void SetScale(glm::vec3 scale);

	glm::vec3 GetScale();

};

