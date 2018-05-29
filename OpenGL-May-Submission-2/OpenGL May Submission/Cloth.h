#pragma once

#include "GlobalIncludes.h"
#include "Point.h"
#include "Spring.h"
#include "Object.h"

class Cloth : public Object
{
public:
	Cloth(float _width, float _height, int _pointDensityX, int _pointDensityY, GLuint _shader);
	~Cloth();

	void CreateVBOAttributes();

	void CreateEBO();

	void GLInit();
	void AddForce(const glm::vec3 _force);
	void Step();
	void Render();

	Point* GetPoint(int _x, int _y);
	void MakeSpring(Point* _point1, Point* _point2);

	glm::vec3 GetTriangleNormal(Point* _point1, Point* _point2, Point* _point3);

private:
	std::vector<Point> m_points;
	std::vector<Spring> m_springs;

	int m_iWidth, m_iHeight;

	// OpenGL
	std::vector<GLuint> m_vecIndices;
	std::vector<Vertice> m_vecVertices;
	GLuint VBO, VAO, EBO;

	//GLuint m_iScaleLocation, m_iRotateLocation, m_iTranslateLocation, m_icurrentTimeLocation;
	//GLuint m_shader;
};

