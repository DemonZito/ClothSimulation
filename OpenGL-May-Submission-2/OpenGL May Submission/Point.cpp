#include "Point.h"

Point::Point(glm::vec3 _pos)
	:
	m_position(_pos),
	m_oldPosition(_pos),
	m_normal(glm::vec3(0,0,0)),
	m_fMass(1.0f),
	m_acceleration(glm::vec3(0,0,0)),
	m_bFixed(false)
{
}

void Point::AddForce(glm::vec3 _force)
{
	m_acceleration += _force/ m_fMass;
}

void Point::Step()
{
	if (!m_bFixed)
	{
		glm::vec3 temp = m_position;
		m_position =  (1.99f * m_position) - m_oldPosition + (m_acceleration * pow(g_kfTimeStep,2));//m_position + (m_position - m_oldPosition) * (1.0f - 0.01f) + m_acceleration * g_kfTimeStep;
		m_oldPosition = temp;
		m_acceleration = glm::vec3(0, 0, 0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)	
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
