#include "Cloth.h"



Cloth::Cloth(int _pointDensityX, int _pointDensityY, int numOfHooks, GLuint _shader, float _stiffness)
	:
	m_fStiffness(_stiffness),
	m_iWidth(_pointDensityX),
	m_iHeight(_pointDensityY),
	Object(_shader)
{
	// Resize vector to dimensions of width and height
	m_points.resize(_pointDensityX * _pointDensityY);

	// Loop through width and height and create a new point at that position
	for (int x = 0; x < _pointDensityX; x++)
	{
		for (int y = 0; y < _pointDensityY; y++)
		{
			m_points[y * _pointDensityX + x] = Point(glm::vec3(_pointDensityX * (x / static_cast<float>(_pointDensityX)),
				-_pointDensityY * (y / static_cast<float>(_pointDensityY)),
				0));
		}
	}

	// Set up triangles and add appropriate points
	int k = 0;
	for (int y = 0; y < (_pointDensityY - 1); y++)
	{
		for (int x = 0; x < (_pointDensityX - 1); x++)
		{
			// Add new triangle to vector
			m_vecTriangles.push_back(new Triangle(_shader, k));
			
			// Add correct points
			m_vecTriangles[k]->AddPoint(&m_points[y * _pointDensityX + x]);
			m_vecTriangles[k]->AddPoint(&m_points[y * _pointDensityX + (x + 1)]);
			m_vecTriangles[k]->AddPoint(&m_points[(y + 1) * _pointDensityX + x]);

			// Init graphics
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

	// Setup spring constraints 
	for (int x = 0; x < _pointDensityX; x++)
	{
		for (int y = 0; y < _pointDensityY; y++)
		{
			// Structural constraints
			if (x < _pointDensityX - 1) MakeSpring(GetPoint(x, y), GetPoint(x + 1, y), 0);
			if (y < _pointDensityY - 1) MakeSpring(GetPoint(x, y), GetPoint(x, y + 1), 0);
			
			// Shear constraints
			if (x < _pointDensityX - 1 && y < _pointDensityY - 1) MakeSpring(GetPoint(x, y), GetPoint(x + 1, y + 1), 0);
			if (x < _pointDensityX - 1 && y < _pointDensityY - 1) MakeSpring(GetPoint(x + 1, y), GetPoint(x, y + 1), 0);
			
		}
	}

	// Hook logic
	if (numOfHooks % 2 == 1)
	{
		--numOfHooks;
		// Add a hook at the very center
		GetPoint((m_iWidth - 1) / 2, 0)->SetFixed(true);
	}

	// Hooks
	int spaceBetweenHooks = 0;
	if(numOfHooks != 1)
		spaceBetweenHooks = glm::ceil(m_iWidth / (numOfHooks - 1));

	// Loop and fix points
	for (int i = 0; i < (numOfHooks / 2); ++i)
	{
		int place = i * spaceBetweenHooks;

		GetPoint(place, 0)->SetFixed(true);

		GetPoint(m_iWidth - 1 - place, 0)->SetFixed(true);
	}
}


Cloth::~Cloth()
{
}

// Add force to all valid points
void Cloth::AddForce(const glm::vec3 _force)
{
	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		std::vector<Point*> points = m_vecTriangles[i]->GetPoints();
		for (auto pt = points.begin(); pt != points.end(); pt++)
		{
			(*pt)->AddForce(_force);
		}
	}
}

void Cloth::FloorCollision()
{
	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		std::vector<Point*> points = m_vecTriangles[i]->GetPoints();
		for (auto pt = points.begin(); pt != points.end(); pt++)
		{
			if ((*pt)->GetPosition().y < -30.0f)
			{
				// If points below floor, repel upwards
				float diff = (*pt)->GetPosition().y - -30;
				(*pt)->ChangePos(glm::vec3(0, 0.05f, 0));
			}
		}
	}
}

// Sphere collision, repel point proportional to penetration depth
void Cloth::ballCollision(const glm::vec3 center, const float radius)
{
	for (auto point = m_points.begin(); point != m_points.end(); point++)
	{
		glm::vec3 v = (*point).GetPosition() - center;
		float l = glm::length(v);

		if (l < radius)
		{
			(*point).ChangePos(glm::normalize(v) * (radius - l));
		}
	}
}

// Get distance
float GetDist(glm::vec3 u, glm::vec3 v)
{
	glm::vec3 diff = u - v;
	return glm::sqrt(glm::dot(diff, diff));
}

// Point and capsule collision
void Cloth::CapsuleCollision(glm::vec3 pt1, glm::vec3 pt2, float lengthsq, float radius)
{
	for (auto point = m_points.begin(); point != m_points.end(); point++)
	{
			glm::vec3 v = pt2 - pt1;
			glm::vec3 w = (*point).GetPosition() - pt1;
			float closestDistance = 0;

			// Check line segment end 1
			float c1 = dot(w, v);
			if (c1 <= 0)
			{
				closestDistance = GetDist((*point).GetPosition(), pt1);

				// Collision occurred
				if (closestDistance < radius)
				{
					// Repell point
					(*point).ChangePos(glm::normalize(w) * (radius - closestDistance));
				}
			}

			// Check line segment end 2
			float c2 = dot(v, v);
			if (c2 <= c1)
			{
				closestDistance = GetDist((*point).GetPosition(), pt2);

				// Collision occurred
				if (closestDistance < radius)
				{
					// Repell point
					(*point).ChangePos(glm::normalize(w) * (radius - closestDistance));
				}
			}
	}
}

void Cloth::pyramidCollision(const glm::vec3 center, Model pyramid)
{
	// Get Triangles from pyramind
	std::vector<std::vector<Vertex>> triangles = pyramid.GetTriangles();

	// For each plane, do a point and plane collision with every point in the cloth
	for (auto point = m_points.begin(); point != m_points.end(); point++)
	{
		bool result = false;
		for (int i = 0; i < triangles.size(); ++i)
		{
			glm::vec3 v = (*point).GetPosition();
			float dProduct = glm::dot(v, triangles.at(i).at(0).Normal) - glm::dot(triangles.at(i).at(0).Normal, (triangles.at(i).at(0).Position + center));
			result = (dProduct > 0);
			if (result) // The point is not behind one of the plains, stop checking for collision
				break;
		}

		// The point is intersecting with the pyramid
		if (!result)
		{
			std::cout << "Pyramid Colliding" << std::endl;

			glm::vec3 v = (*point).GetPosition() - center;
			float l = glm::length(v);

			// Repell point
			(*point).ChangePos(glm::normalize(v) * (2.0f - l));
		}
	}
}

// Update method
void Cloth::Step()
{	
	// Satisfy all constraints
	for (int i = 0; i < g_kiConstraintIterations; i++)
	{
		for (auto constraint = m_springs.begin(); constraint != m_springs.end(); constraint++) {
			(*constraint).SatisfyConstraint();
		}
	}

	// Update points and break those which are overextended
	for (int x = 0; x < m_iWidth; x++)
	{
		for (int y = 0; y < m_iHeight; y++)
		{
			GetPoint(x, y)->Step();
			if (GetPoint(x, y)->m_bOverExtended)
			{
				Tear(GetPoint(x, y));
			}
		}
	}

	// Update all detached points
	for (auto point = m_detachedPoints.begin(); point != m_detachedPoints.end(); point++)
	{
		(*point)->Step();
	}

	// Update triangles and graphics of vertices based on the points
	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		m_vecTriangles[i]->Step();
	}
}

void Cloth::Render()
{
	// reset light normals
	for (auto point = m_points.begin(); point != m_points.end(); point++)
	{
		(*point).ResetNormals();
	}

	// Update normals and fix lighting each frame
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

	// Render each triangle
	for (int i = 0; i < m_vecTriangles.size(); i++)
	{
		m_vecTriangles[i]->Render();
	}
}

// Linearly interpolate
float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

// Lerp curtain hooks closer or further away from eachother
void Cloth::MoveCurtain(float direction)
{
	for (auto point = m_points.begin(); point != m_points.end(); point++)
	{
		if (point->GetFixed() && point->GetPosition().x != 0)
		{
			glm::vec3 pos = point->GetPosition();
			point->SetPos(glm::vec3(lerp(pos.x, 0.0f, direction), pos.y, pos.z));
		}
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

// Create constraint here, add links for self collision
void Cloth::MakeSpring(Point* _point1, Point* _point2, float _restingDistance)
{
	 m_springs.push_back(Spring(_point1, _point2, _restingDistance, m_fStiffness));
	 _point1->AddLink(_point2);
	 _point2->AddLink(_point1);
}


// Detach point here
void Cloth::Tear(Point* _pt)
{
	// If not already detached
	if (!_pt->GetDetached())
	{
		_pt->m_bOverExtended = false;

		// Loop through each triangle this point belongs to and remove it
		for (int triangle = 0; triangle < _pt->m_iTriangleIdx.size(); triangle++)
		{
			Point temp = *_pt;
			std::vector<Point*> points = m_vecTriangles[_pt->m_iTriangleIdx[triangle]]->GetPoints();

			// Remove the point
			for (int i = 0; i < 3; i++)
			{
				if (points[i] == _pt)
				{
					points.erase(points.begin() + i);
					break;
				}
			}

			// Create new detached point
			Point* newPoint = new Point(temp.GetPosition());
			newPoint->SetDetached(true);
			newPoint->m_iTriangleIdx = temp.m_iTriangleIdx;

			// Fix constraints between this triangle
			if (!points[0]->GetDetached()) { MakeSpring(points[0], newPoint, 1.0f); }
			if (!points[1]->GetDetached()) { MakeSpring(points[1], newPoint, 1.0f); }

			if (points[0]->GetDetached() && points[1]->GetDetached())
			{
				MakeSpring(points[0], newPoint, 1.0f);
				MakeSpring(points[1], newPoint, 1.0f);
			}

			// Readd the point to the triangle vector
			points.push_back(newPoint);

			// Reset points and fix triangle
			m_vecTriangles[_pt->m_iTriangleIdx[triangle]]->ResetPoints();
			for (int i = 0; i < 3; i++)
			{
				m_vecTriangles[_pt->m_iTriangleIdx[triangle]]->AddPoint(points[i]);
			}

			// Add new point to detached points
			m_detachedPoints.push_back(newPoint);

			// Break old and now invalid constraints to this point
			for (auto constraint = m_springs.begin(); constraint != m_springs.end(); constraint++) {
				if ((*constraint).m_pPoint1 == _pt)
				{
					(*constraint).m_bBroken = true;
				}

				if ((*constraint).m_pPoint2 == _pt)
				{
					(*constraint).m_bBroken = true;
				}
			}

		}

	}
}

void Cloth::PushCloth(Triangle* _tri, glm::vec3 _direction)
{
	// Push all points in the triangle
	std::vector<Point*> points = _tri->GetPoints();
	points[0]->AddForce(_direction * 10.0f);
	points[1]->AddForce(_direction * 10.0f);
	points[2]->AddForce(_direction * 10.0f);

}

// Add wind to all points
void Cloth::windForce(const glm::vec3 direction)
{
	for (int i = 0; i < m_vecTriangles.size(); ++i)
	{
		addWindForcesForTriangle(m_vecTriangles.at(i), direction);
	}
}

// Calculate normal for lighting
glm::vec3 Cloth::calcTriangleNormal(Point * _p0, Point * _p1, Point * _p2)
{
	glm::vec3 pos1 = _p0->GetPosition();
	glm::vec3 pos2 = _p1->GetPosition();
	glm::vec3 pos3 = _p2->GetPosition();

	glm::vec3 v1 = pos2 - pos1;
	glm::vec3 v2 = pos3 - pos1;

	return glm::cross(v1, v2);
}

// Add wind forces to each triangle
void Cloth::addWindForcesForTriangle(Triangle* triangle, const glm::vec3 direction)
{
	std::vector<Point*> points = triangle->GetPoints();
	glm::vec3 normal = calcTriangleNormal(points[0], points[1], points[2]);
	glm::vec3 d = glm::normalize(normal);
	glm::vec3 force = normal * (glm::dot(d, direction));
	points[0]->AddForce(force);
	points[1]->AddForce(force);
	points[2]->AddForce(force);
}


glm::vec3 Cloth::CalculateTriangleNormal(Point * _p0, Point * _p1, Point * _p2)
{
	glm::vec3 v1 = _p1->GetPosition() - _p0->GetPosition();
	glm::vec3 v2 = _p2->GetPosition() - _p0->GetPosition();

	return glm::cross(v1, v2);
}

// Adjust stiffness of all constraints
void Cloth::ChangeStiffness(float _stiffness)
{
	m_fStiffness = _stiffness;
	for (auto constraint = m_springs.begin(); constraint != m_springs.end(); constraint++) {
		(*constraint).m_fStiffness = _stiffness;
	}
}

void Cloth::SelfCollision()
{
	// Cycle over all triangles
	for (int i = 0; i < m_vecTriangles.size(); ++i)
	{
		// Cycle over each point in a triangle
		std::vector<Point*> points = m_vecTriangles.at(i)->GetPoints();
		for (int j = 0; j < m_vecTriangles.at(i)->GetPoints().size(); ++j)
		{
			if (!points.at(j)->GetDetached())
			{
				// Cycle over all points in cloth
				for (auto point = m_points.begin(); point != m_points.end(); point++)
				{
					// Check it isnt itself
					if (points.at(j) == &(*point))
						continue;

					bool isConnected = false;

					// Check it isnt a point connected to self
					std::vector<Point*> links = points.at(j)->GetLinks();

					// Check if this is a link to this point, if so, don't collide
					for (int k = 0; k < links.size(); ++k)
					{
						if (links.at(k) == &(*point))
						{
							isConnected = true;
						}
					}
					if (isConnected)
						continue;

					// Do sphere collision
					glm::vec3 v = points.at(j)->GetPosition() - point->GetPosition();
					float l = glm::length(v);

					if (l < m_fPointRadius)
					{
						points.at(j)->ChangePos(glm::normalize(v) * (m_fPointRadius - l));
					}
				}
			}
			
		}
	}
}
