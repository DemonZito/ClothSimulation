#pragma once

#include "GlobalIncludes.h"
#include "Point.h"
#include "Object.h"

class Triangle : public Object
{
public:
	Triangle(GLuint _shader, int _idx);
	~Triangle();

	//void AddVertex(Vertice _v);

	void AddPoint(Point * _p);

	void CreateVBOAttributes();
	void CreateEBO();
	void GLInit();

	std::vector<Point*> GetPoints();
	void ResetPoints() { m_vecPoints.clear(); }

	bool GetDetached();

	void SetDetached(bool _isDetached);

	bool Step();
	void Render();

private: 
	std::vector<Point*> m_vecPoints;
	GLuint VBO, VAO, EBO;
	std::vector<Vertice> m_vecVertices;
	bool m_bDetached = false;
	int m_iTriangleIdx;
	bool m_bBurning = false;
};

