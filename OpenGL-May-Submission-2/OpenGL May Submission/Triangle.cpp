#include "Triangle.h"



Triangle::Triangle(GLuint _shader, int _idx)
	:
	Object(_shader)
{
	m_iTriangleIdx = _idx;
}


Triangle::~Triangle()
{
}

//void Triangle::AddVertex(Vertice _v)
//{
//	m_vecVertices.push_back(_v);
//}

void Triangle::AddPoint(Point* _p)
{
	_p->m_iTriangleIdx.push_back(this->m_iTriangleIdx);
	//_p->SetDetached(false);
	m_vecPoints.push_back(_p);

	if (m_vecVertices.size() < 3)
	{
		Vertice newVertex;
		newVertex.Position = _p->GetPosition();
		newVertex.Color = glm::vec3(0.8f, 0.3f, 0.5f);
		newVertex.NormalCoord = glm::vec3(0, 0, 0);
		newVertex.TexCoord = glm::vec2(0, 0);
		m_vecVertices.push_back(newVertex);

	}

}

void Triangle::CreateVBOAttributes()
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 11 * m_vecVertices.size(), m_vecVertices.data(), GL_DYNAMIC_DRAW);

	GLuint aPositionLocation = glGetAttribLocation(m_shader, "a_position");
	glVertexAttribPointer(aPositionLocation, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(aPositionLocation);

	GLuint aColor = glGetAttribLocation(m_shader, "a_color");
	glVertexAttribPointer(aColor, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), BUFFER_OFFSET(3));
	glEnableVertexAttribArray(aColor);

	GLuint aNormal = glGetAttribLocation(m_shader, "a_normal");
	glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), BUFFER_OFFSET(6));
	glEnableVertexAttribArray(aNormal);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Triangle::CreateEBO()
{
	std::vector<GLuint> indicies = { 0,1,2 };

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.data(), GL_DYNAMIC_DRAW);
}

void Triangle::GLInit()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	CreateVBOAttributes();
	CreateEBO();

	GetUniformLocation();
}

std::vector<Point*> Triangle::GetPoints()
{
	return m_vecPoints;
}


bool Triangle::GetDetached()
{
	return m_bDetached;
}

void Triangle::SetDetached(bool _isDetached)
{
	m_bDetached = _isDetached;
}

bool Triangle::Step()
{
	bool breakage = true;
	for (int i = 0; i < 3; i++)
	{
		m_vecVertices[i].Position = m_vecPoints[i]->GetPosition();
		/*if (m_vecPoints[i]->GetDetached())
		{
			Point* newPoint = new Point(m_vecPoints[i]->GetPosition());
			newPoint->SetLinks(m_vecPoints[i]->GetLinks());
			newPoint->SetDetached(true);
			newPoint->m_iTriangleIdx.push_back(m_iTriangleIdx);
			m_vecPoints[i] = newPoint;
			breakage =  false;
		}*/
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 11 * m_vecVertices.size(), m_vecVertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return breakage;
}

void Triangle::Render()
{
	for (int i = 0; i < 3; i++)
	{
		m_vecVertices[i].NormalCoord = m_vecPoints[i]->GetNormal();
	}
	
	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 Translate = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

	glUniformMatrix4fv(m_iScaleLocation, 1, GL_FALSE, glm::value_ptr(Scale));
	glUniformMatrix4fv(m_iRotateLocation, 1, GL_FALSE, glm::value_ptr(Rotate));
	glUniformMatrix4fv(m_iTranslateLocation, 1, GL_FALSE, glm::value_ptr(Translate));

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}
