#include "Point.h"

Point::Point(glm::vec3 _pos)
	:
	m_position(_pos),
	m_oldPosition(_pos),
	m_normal(glm::vec3(0, 0, 0)),
	m_fMass(1.0f),
	m_acceleration(glm::vec3(0, 0, 0)),
	m_bFixed(false)
{
}

void Point::AddForce(glm::vec3 _force)
{
	m_acceleration += _force / m_fMass;
}

void Point::Step()
{
	if (!m_bFixed)
	{
		glm::vec3 temp = m_position;
		m_position = m_position + 0.99f * (m_position - m_oldPosition) + (m_acceleration * pow(g_kfTimeStep, 2));//m_position + (m_position - m_oldPosition) * (1.0f - 0.01f) + m_acceleration * g_kfTimeStep;
		m_oldestPosition = m_oldPosition;
		m_oldPosition = temp;
		m_acceleration = glm::vec3(0, 0, 0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)	
	}

	if (m_bBurning && m_fBurnMeter > 0)
		m_fBurnMeter -= GenerateRandomFloat(0.0f, 0.01f);
	else if (!m_bOverExtended && !m_bDetached && m_bBurning && m_fBurnMeter <= 0)
	{
		m_bOverExtended = true;
		m_bBurning = false;
	}
}

void Point::ChangePos(const glm::vec3 _delta)
{
	if (!m_bFixed) m_position += _delta;
}

void Point::SetFixed(bool _isFixed)
{
	m_bFixed = _isFixed;
}

bool Point::GetFixed()
{
	return m_bFixed;
}

void Point::ChangeNormal(glm::vec3 _delta)
{
	m_normal += glm::normalize(_delta);
}

glm::vec3 & Point::GetPosition()
{
	return m_position;
}

glm::vec3 & Point::GetNormal()
{
	return m_normal;
}

float Point::GetMass()
{
	return m_fMass;
}

glm::vec2 Point::GetIndex()
{
	return glm::vec2(m_iX, m_iY);
}

std::vector<Point*> Point::GetLinks()
{
	return m_vecLinks;
}

void Point::SetLinks(std::vector<Point*> _links)
{
	m_vecLinks = _links;
}

void Point::AddLink(Point* _link)
{
	m_vecLinks.push_back(_link);
}

void Point::RemoveLink(Point* _link)
{
	for (int i = 0; i < m_vecLinks.size();)
	{
		if(m_vecLinks[i] == _link)
		{
			m_vecLinks.erase(m_vecLinks.begin() + i);
		}
		else
		{
			i++;
		}
	}
}