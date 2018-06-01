#pragma once

#include "GlobalIncludes.h"

class Point
{
public:
	Point(glm::vec3 _pos);
	Point(){}

	void AddForce(glm::vec3 _force);
	void Step();
	void ChangePos(const glm::vec3 _delta);
	void SetFixed(bool _isFixed);
	void ChangeNormal(glm::vec3 _delta);

	glm::vec3& GetPosition();
	glm::vec3& GetNormal();
	float GetMass();

	void ResetNormals() { m_normal = glm::vec3(0, 0, 0); }

private:
	glm::vec3 m_position;
	glm::vec3 m_oldPosition;
	glm::vec3 m_acceleration;
	float m_fMass;
	glm::vec3 m_normal;

	bool m_bFixed;
};

