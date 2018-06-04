#include "Cloth.h"



Cloth::Cloth(int _pointDensityX, int _pointDensityY, GLuint _shader)
	:
	m_iWidth(_pointDensityX),
	m_iHeight(_pointDensityY),
	Object(_shader)
{
	m_points.resize(_pointDensityX * _pointDensityY);

	for (int x = 0; x < _pointDensityX; x++)
	{
		for (int y = 0; y < _pointDensityY; y++)
		{
			m_points[y * _pointDensityX + x] = Point(glm::vec3(_pointDensityX * (x / static_cast<float>(_pointDensityX)),
				-_pointDensityY * (y / static_cast<float>(_pointDensityY)),
				0));
		}
	}

	int k = 0;
	for (int y = 0; y < (_pointDensityX - 1); y++)
	{
		for (int x = 0; x < (_pointDensityX - 1); x++)
		{
			m_vecTriangles.push_back(new Triangle(_shader, k));
			m_vecTriangles[k]->AddPoint(&m_points[y * _pointDensityX + x]);
			m_vecTriangles[k]->AddPoint(&m_points[y * _pointDensityX + (x + 1)]);
			m_vecTriangles[k]->AddPoint(&m_points[(y + 1) * _pointDensityX + x]);
			m_vecTriangles[k]->GLInit();
			k++;

			m_vecTriangles.push_back(new Triangle(_shader, k));
			m_vecTriangles[k]->AddPoint(&m_points[(y + 1) * _pointDensityX + x]);
			m_vecTriangles[k]->AddPoint(&m_points[y * _pointDensityX + (x + 1)]);
			m_vecTriangles[k]->AddPoint(&m_points[(y + 1) * _pointDensityX + (x + 1)]);
			m_vecTriangles[k]->GLInit();
			k++;
		}
	}

	for (int x = 0; x < _pointDensityX; x++)
	{
		for (int y = 0; y < _pointDensityY; y++)
		{
			if (x < _pointDensityX - 1)
			{
				MakeSpring(GetPoint(x, y), GetPoint(x + 1, y));
				//GetPoint(x, y)->AddLink(GetPoint(x + 1, y));
				//GetPoint(x + 1, y)->AddLink(GetPoint(x, y));
			}
			if (y < _pointDensityY - 1)
			{
				MakeSpring(GetPoint(x, y), GetPoint(x, y + 1));
				//GetPoint(x, y)->AddLink(GetPoint(x, y + 1));
				//GetPoint(x, y + 1)->AddLink(GetPoint(x, y));

			}
			if (x < _pointDensityX - 1 && y < _pointDensityY - 1)
			{
				MakeSpring(GetPoint(x, y), GetPoint(x + 1, y + 1));
				//GetPoint(x, y)->AddLink(GetPoint(x + 1, y + 1));
				//GetPoint(x + 1, y + 1)->AddLink(GetPoint(x, y));

			}
			if (x < _pointDensityX - 1 && y < _pointDensityY - 1)
			{
				MakeSpring(GetPoint(x + 1, y), GetPoint(x, y + 1));
				//GetPoint(x + 1, y)->AddLink(GetPoint(x, y + 1));
				//GetPoint(x, y + 1)->AddLink(GetPoint(x + 1, y));

			}
		}
	}

	for (int x = 0; x<_pointDensityX; x++)
	{
		for (int y = 0; y<_pointDensityY; y++)
		{
			if (x < _pointDensityX - 2)
			{
				MakeSpring(GetPoint(x, y), GetPoint(x + 2, y));
				//GetPoint(x, y)->AddLink(GetPoint(x + 2, y));
				//GetPoint(x + 2, y)->AddLink(GetPoint(x, y));

			}
			if (y < _pointDensityY - 2)
			{
				MakeSpring(GetPoint(x, y), GetPoint(x, y + 2));
				//GetPoint(x, y)->AddLink(GetPoint(x, y + 2));
				//GetPoint(x, y + 2)->AddLink(GetPoint(x, y));

			}
			if (x < _pointDensityX - 2 && y < _pointDensityY - 2)
			{
				MakeSpring(GetPoint(x, y), GetPoint(x + 2, y + 2));
				//GetPoint(x, y)->AddLink(GetPoint(x + 2, y + 2));
				//GetPoint(x + 2, y + 2)->AddLink(GetPoint(x, y));

			}
			if (x < _pointDensityX - 2 && y < _pointDensityY - 2)
			{
				MakeSpring(GetPoint(x + 2, y), GetPoint(x, y + 2));
				//GetPoint(x + 2, y)->AddLink(GetPoint(x, y + 2));
				//GetPoint(x, y + 2)->AddLink(GetPoint(x + 2, y));
			}
		}
	}



	GetPoint(1, 0)->ChangePos(glm::vec3(0.0f, 0.0f, 0.0f));
	GetPoint(1, 0)->SetFixed(true);
	GetPoint(m_iWidth - 1, 0)->SetFixed(true);
}


Cloth::~Cloth()
{
}


void Cloth::AddForce(const glm::vec3 _force)
{
	//for (auto point = m_points.begin(); point != m_points.end(); point++) {
	//	(*point).AddForce(_force);
	//}

	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		std::vector<Point*> points = m_vecTriangles[i]->GetPoints();
		for (auto pt = points.begin(); pt != points.end(); pt++)
		{
			(*pt)->AddForce(_force);
		}
	}
}

void Cloth::MoveClothPoint(glm::vec3 delta)
{
	GetPoint(1, 0)->SetFixed(false);
	GetPoint(1, 0)->ChangePos(delta);
	GetPoint(1, 0)->SetFixed(true);

	GetPoint(m_iWidth - 1, 0)->SetFixed(false);
	GetPoint(m_iWidth - 1, 0)->ChangePos(-delta);
	GetPoint(m_iWidth - 1, 0)->SetFixed(true);
}

void Cloth::Step()
{	
	for (int i = 0; i < g_kiConstraintIterations; i++)
	{
		for (auto constraint = m_springs.begin(); constraint != m_springs.end(); constraint++) {
			(*constraint).SatisfyConstraint();
		}
	}

	for (int x = 0; x < m_iWidth; x++)
	{
		for (int y = 0; y < m_iHeight; y++)
		{
			// Update point
			//if (!GetPoint(x, y)->GetDetached())
			//{
				GetPoint(x, y)->Step();
			//}

			// Update visuals of that point
		}
	}

	for (auto point = m_detachedPoints.begin(); point != m_detachedPoints.end(); point++)
	{
		//if (!(*point)->GetDetached())
		//{
			(*point)->Step();
		//}
	}

	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		if(!m_vecTriangles[i]->Step())
		{
			std::vector<Point*> points = m_vecTriangles[i]->GetPoints();

			for (int ptIdx = 0; ptIdx < 3; ptIdx++)
			{
				if (points[ptIdx]->GetDetached())
				{
					m_detachedPoints.push_back(points[ptIdx]);
					m_detachedPoints[m_detachedPoints.size() - 1]->SetDetached(false);

					std::vector<Point*> links = m_detachedPoints[m_detachedPoints.size() - 1]->GetLinks();
					for (int connection = 0; connection < links.size(); connection++)
					{
						if (!links[connection]->GetDetached())
						{
							MakeSpring(m_detachedPoints[m_detachedPoints.size() - 1], links[connection]);
							//m_detachedPoints[m_detachedPoints.size() - 1]->AddLink(links[connection]);
							//links[connection]->AddLink(m_detachedPoints[m_detachedPoints.size() - 1]);

						}
					}
				}
			}
		}
	}
}

void Cloth::Render()
{
	for (auto point = m_points.begin(); point != m_points.end(); point++)
	{
		(*point).ResetNormals();
	}

	//create smooth per particle normals by adding up all the (hard) triangle normals that each particle is part of
	for (int x = 0; x<m_iWidth - 1; x++)
	{
		for (int y = 0; y<m_iHeight - 1; y++)
		{
			glm::vec3 normal = CalculateTriangleNormal(GetPoint(x + 1, y), GetPoint(x, y), GetPoint(x, y + 1));
			GetPoint(x + 1, y)->ChangeNormal(normal);
			GetPoint(x, y)->ChangeNormal(normal);
			GetPoint(x, y + 1)->ChangeNormal(normal);

			normal = CalculateTriangleNormal(GetPoint(x + 1, y + 1), GetPoint(x + 1, y), GetPoint(x, y + 1));
			GetPoint(x + 1, y + 1)->ChangeNormal(normal);
			GetPoint(x + 1, y)->ChangeNormal(normal);
			GetPoint(x, y + 1)->ChangeNormal(normal);
		}
	}

	glUseProgram(m_shader);

	//

	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		m_vecTriangles[i]->Render();
	}
}

std::vector<Triangle*> Cloth::GetTriangles()
{
	return m_vecTriangles;
}

Point* Cloth::GetPoint(int _x, int _y)
{
	return &m_points[_y * m_iWidth + _x]; 
}

void Cloth::MakeSpring(Point* _point1, Point* _point2)
{
	 m_springs.push_back(Spring(_point1, _point2));
	 _point1->AddLink(_point2);
	 _point2->AddLink(_point1);
}

void Cloth::PushCloth(int _idx, Point* _pt, glm::vec3 _direction)
{
	if (!_pt->GetDetached())
	{
		Point temp = *_pt;
		std::vector<Point*> points = m_vecTriangles[_idx]->GetPoints();

		for (int i = 0; i < 3; i++)
		{
			if (points[i] == _pt)
			{
				points.erase(points.begin() + i);
				break;
			}
		}

		Point* newPoint = new Point(temp.GetPosition());
		newPoint->SetDetached(true);

		if (!points[0]->GetDetached())
			//MakeSpring(points[0], newPoint);

		if (!points[1]->GetDetached())
			//MakeSpring(points[1], newPoint);

		if (points[0]->GetDetached() && points[1]->GetDetached())
		{
			MakeSpring(points[0], newPoint);
			MakeSpring(points[1], newPoint);
		}

		points.push_back(newPoint);

		m_vecTriangles[_idx]->ResetPoints();
		for (int i = 0; i < 3; i++)
		{
			m_vecTriangles[_idx]->AddPoint(points[i]);
		}

		m_detachedPoints.push_back(newPoint);

	}
	
}

glm::vec3 Cloth::CalculateTriangleNormal(Point * _p0, Point * _p1, Point * _p2)
{
	glm::vec3 v1 = _p1->GetPosition() - _p0->GetPosition();
	glm::vec3 v2 = _p2->GetPosition() - _p0->GetPosition();

	return glm::cross(v1, v2);
}
