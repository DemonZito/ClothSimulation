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
	void SetPos(glm::vec3 _newPos) { m_position = _newPos; }
	void SetFixed(bool _isFixed);
	bool GetFixed();
	void ChangeNormal(glm::vec3 _delta);
	void SetDetached(bool _isDetached) { m_bDetached = _isDetached; }
	bool GetDetached() { return m_bDetached; }

	glm::vec3& GetPosition();
	glm::vec3& GetNormal();
	float GetMass();

	void ResetNormals() { m_normal = glm::vec3(0, 0, 0); }

	glm::vec2 GetIndex();

	std::vector<Point*> GetLinks();
	void SetLinks(std::vector<Point*> _links);
	void AddLink(Point* _link);
	void RemoveLink(Point* _link);

	std::vector<int> m_iTriangleIdx;

private:
	glm::vec3 m_position;
	glm::vec3 m_oldPosition;
	glm::vec3 m_acceleration;
	float m_fMass;
	glm::vec3 m_normal;

	int m_iX, m_iY;

	bool m_bDetached = false;
	bool m_bFixed;
	
	std::vector<Point*> m_vecLinks;
};

