#include "Cloth.h"



Cloth::Cloth(int _pointDensityX, int _pointDensityY, GLuint _shader)
	:
	m_iWidth(_pointDensityX),
	m_iHeight(_pointDensityY),
	Object(_shader)
{
	m_vecTriangles.resize(_pointDensityX * _pointDensityY);
	m_points.resize(_pointDensityX * _pointDensityY);

	for (int x = 0; x < _pointDensityX; x++)
	{
		for (int y = 0; y < _pointDensityY; y++)
		{
			m_points[y * _pointDensityX + x] = Point(glm::vec3(_pointDensityX * (x / static_cast<float>(_pointDensityX)),
				-_pointDensityY * (y / static_cast<float>(_pointDensityY)),
				0));

			Vertice curVertex;
			curVertex.Position = m_points[y * _pointDensityX + x].GetPosition();
			curVertex.Color = glm::vec3(1.0f, 1.0f, 0.0f);
			curVertex.TexCoord = glm::vec2(x / _pointDensityX, y / _pointDensityY);
			m_vecVertices.push_back(curVertex);
		}
	}

	for (int x = 0; x < (_pointDensityX - 1); x++) {
		for (int y = 0; y < (_pointDensityY - 1); y++) {
			for (int k = 0; k < 6; k++) {
				m_vecIndices.push_back(0);
			}
		}
	}

	int k = 0;
	for (int y = 0; y < (_pointDensityX - 1); y++)
	{
		for (int x = 0; x < (_pointDensityX - 1); x++)
		{
			m_vecIndices[k] = (y * _pointDensityX + x);
			m_vecIndices[k + 1] = (y * _pointDensityX + (x + 1));
			m_vecIndices[k + 2] = ((y + 1) * _pointDensityX + x);

			m_vecIndices[k + 3] = ((y + 1) * _pointDensityX + x);
			m_vecIndices[k + 4] = (y * _pointDensityX + (x + 1));
			m_vecIndices[k + 5] = ((y + 1) * _pointDensityX + (x + 1));

			k += 6;
		}
	}

	for (int x = 0; x < _pointDensityX; x++)
	{
		for (int y = 0; y < _pointDensityY; y++)
		{
			if (x<_pointDensityX - 1) MakeSpring(GetPoint(x, y), GetPoint(x + 1, y));
			if (y<_pointDensityY - 1) MakeSpring(GetPoint(x, y), GetPoint(x, y + 1));
			if (x<_pointDensityX - 1 && y<_pointDensityY - 1) MakeSpring(GetPoint(x, y), GetPoint(x + 1, y + 1));
			if (x<_pointDensityX - 1 && y<_pointDensityY - 1) MakeSpring(GetPoint(x + 1, y), GetPoint(x, y + 1));
		}
	}

	for (int x = 0; x<_pointDensityX; x++)
	{
		for (int y = 0; y<_pointDensityY; y++)
		{
			if (x<_pointDensityX - 2) MakeSpring(GetPoint(x, y), GetPoint(x + 2, y));
			if (y<_pointDensityY - 2) MakeSpring(GetPoint(x, y), GetPoint(x, y + 2));
			if (x<_pointDensityX - 2 && y<_pointDensityY - 2) MakeSpring(GetPoint(x, y), GetPoint(x + 2, y + 2));
			if (x<_pointDensityX - 2 && y<_pointDensityY - 2) MakeSpring(GetPoint(x + 2, y), GetPoint(x, y + 2));
		}
	}



	GetPoint(1, 0)->ChangePos(glm::vec3(0.5f, 0.0f, 0.0f));
	GetPoint(1, 0)->SetFixed(true);
	GetPoint(m_iWidth - 1, 0)->SetFixed(true);

	GLInit();
}


Cloth::~Cloth()
{
}

void Cloth::CreateVBOAttributes()
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * m_vecVertices.size(), m_vecVertices.data(), GL_DYNAMIC_DRAW);

	GLuint aPositionLocation = glGetAttribLocation(m_shader, "a_position");
	glVertexAttribPointer(aPositionLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(aPositionLocation);

	GLuint aColor = glGetAttribLocation(m_shader, "a_color");
	glVertexAttribPointer(aColor, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), BUFFER_OFFSET(3));
	glEnableVertexAttribArray(aColor);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Cloth::CreateEBO()
{
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_vecIndices.size(), m_vecIndices.data(), GL_DYNAMIC_DRAW);
}

void Cloth::GLInit()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	CreateVBOAttributes();
	CreateEBO();

	GetUniformLocation();
}

void Cloth::AddForce(const glm::vec3 _force)
{
	for (auto point = m_points.begin(); point != m_points.end(); point++) {
		(*point).AddForce(_force);
	}
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
			GetPoint(x,y)->Step();

			// Update visuals of that point
			m_vecVertices[y * m_iWidth + x].Position = GetPoint(x, y)->GetPosition();

			if (x == 0 && y == 1)
			{
				std::cout << m_vecVertices[y * m_iWidth + x].Position.x << "," << m_vecVertices[y * m_iWidth + x].Position.y << "," << m_vecVertices[y * m_iWidth + x].Position.z << std::endl;
				std::cout << GetPoint(x, y)->GetPosition().x << std::endl;
			}
		}
	}

	m_vecTriangles.clear();
	// Update triangles
	for(int i = 0; i < m_vecVertices.size()/4; i++)
	{
		Triangle newTriangle;
		newTriangle.P0 = m_vecVertices[(i * 3) + 0].Position;
		newTriangle.P1 = m_vecVertices[(i * 3) + 1].Position;
		newTriangle.P2 = m_vecVertices[(i * 3) + 2].Position;
		m_vecTriangles.push_back(newTriangle);

		newTriangle.P0 = m_vecVertices[(i * 3) + 2].Position;
		newTriangle.P1 = m_vecVertices[(i * 3) + 3].Position;
		newTriangle.P2 = m_vecVertices[(i * 3) + 0].Position;
		m_vecTriangles.push_back(newTriangle);
	}


	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8 * m_vecVertices.size(), m_vecVertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Cloth::Render()
{
	glUseProgram(m_shader);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	glUniformMatrix4fv(m_iScaleLocation, 1, GL_FALSE, glm::value_ptr(Scale));
	glUniformMatrix4fv(m_iRotateLocation, 1, GL_FALSE, glm::value_ptr(Rotate));
	glUniformMatrix4fv(m_iTranslateLocation, 1, GL_FALSE, glm::value_ptr(Translate));
	
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, m_vecIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

std::vector<Triangle> Cloth::GetTriangles()
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
}

Triangle Cloth::MakeTriangle(Point * _point1, Point * _point2, Point * _point3)
{
	return Triangle(_point1->GetPosition(), _point2->GetPosition(), _point3->GetPosition());
}
