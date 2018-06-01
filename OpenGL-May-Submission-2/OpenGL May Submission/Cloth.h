#pragma once

#include "GlobalIncludes.h"
#include "Point.h"
#include "Spring.h"
#include "Object.h"

struct Triangle
{
	std::vector<Point*> m_vecPoints;
};

class Cloth : public Object
{
public:
	Cloth(int _pointDensityX, int _pointDensityY, GLuint _shader);
	~Cloth();

	void CreateVBOAttributes();

	void CreateEBO();

	void GLInit();
	void AddForce(const glm::vec3 _force);
	void MoveClothPoint(glm::vec3 delta);
	void Step();
	void Render();

	std::vector<Triangle> GetTriangles();
	Point* GetPoint(int _x, int _y);
	void MakeSpring(Point* _point1, Point* _point2);
	void PushCloth(int _idx, glm::vec3 _direction);
	glm::vec3 CalculateTriangleNormal(Point *_p0, Point *_p1, Point* _p2);

private:
	std::vector<Point> m_points;
	std::vector<Spring> m_springs;

	int m_iWidth, m_iHeight;

	// OpenGL
	std::vector<GLuint> m_vecIndices;
	std::vector<Vertice> m_vecVertices;
	std::vector<Triangle> m_vecTriangles;
	GLuint VBO, VAO, EBO;

	//GLuint m_iScaleLocation, m_iRotateLocation, m_iTranslateLocation, m_icurrentTimeLocation;
	//GLuint m_shader;
};

