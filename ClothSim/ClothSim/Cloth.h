#pragma once

#include "GlobalIncludes.h"
#include "Point.h"
#include "Spring.h"
#include "Object.h"
#include "Triangle.h"

class Cloth : public Object
{
public:
	Cloth(int _pointDensityX, int _pointDensityY, int numOfHooks, GLuint _shader, float _stiffness);
	~Cloth();

	void AddForce(const glm::vec3 _force);
	void FloorCollision();
	void ballCollision(const glm::vec3 center, const float radius);
	void CapsuleCollision(glm::vec3 pt1, glm::vec3 pt2, float lengthsq, float radius_sq);
	void pyramidCollision(const glm::vec3 center, Model pyramid);
	void Step();
	void Render();

	void MoveCurtain(float direction);

	std::vector<Triangle*> GetTriangles();
	Point* GetPoint(int _x, int _y);
	void MakeSpring(Point* _point1, Point* _point2, float _restingDistance);
	void Tear(Point * _pt);
	void PushCloth(Triangle* _tri, glm::vec3 _direction);
	void windForce(const glm::vec3 direction);
	glm::vec3 CalculateTriangleNormal(Point *_p0, Point *_p1, Point* _p2);
	void ChangeStiffness(float _stiffness);
	void SelfCollision();

private:
	glm::vec3 calcTriangleNormal(Point * _p0, Point * _p1, Point * _p2);
	void addWindForcesForTriangle(Triangle * triangle, const glm::vec3 direction);

private:
	std::vector<Point> m_points;
	std::vector<Point*> m_detachedPoints;
	std::vector<Spring> m_springs;

	int m_iWidth, m_iHeight;
	float m_fStiffness = 1.0f;
	float m_fPointRadius = 0.8f;

	// OpenGL
	std::vector<Triangle*> m_vecTriangles;
};

