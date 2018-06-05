#pragma once

#include "GlobalIncludes.h"
#include "Point.h"
#include "Spring.h"
#include "Object.h"
#include "Triangle.h"

class Cloth : public Object
{
public:
	Cloth(int _pointDensityX, int _pointDensityY, int numOfHooks, GLuint _shader);
	~Cloth();

	void AddForce(const glm::vec3 _force);
	void MoveClothPoint(glm::vec3 delta);
	void Step();
	void Render();

	std::vector<Triangle*> GetTriangles();
	Point* GetPoint(int _x, int _y);
	void MakeSpring(Point* _point1, Point* _point2);
	void PushCloth(Point* _pt, glm::vec3 _direction);
	void windForce(const glm::vec3 direction);
	glm::vec3 CalculateTriangleNormal(Point *_p0, Point *_p1, Point* _p2);

private:
	glm::vec3 calcTriangleNormal(Point * _p0, Point * _p1, Point * _p2);
	void addWindForcesForTriangle(Triangle * triangle, const glm::vec3 direction);

private:
	std::vector<Point> m_points;
	std::vector<Point*> m_detachedPoints;
	std::vector<Spring> m_springs;

	int m_iWidth, m_iHeight;

	// OpenGL
	std::vector<Triangle*> m_vecTriangles;
};

