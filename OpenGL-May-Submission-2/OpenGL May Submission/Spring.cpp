#include "Spring.h"

Spring::Spring(Point * _point1, Point * _point2, float _restingDistance, float _stiffness)
	:
	m_pPoint1(_point1),
	m_pPoint2(_point2)
{
	m_fStiffness = _stiffness;

	// Use parameter distance
	if (_restingDistance > 0.1)
		m_fRestDistance = _restingDistance;
	else
	{
		// Generate rest distance
		glm::vec3 vector = _point1->GetPosition() - _point2->GetPosition();
		m_fRestDistance = glm::length(vector);
	}
}

void Spring::SatisfyConstraint()
{
	if (!m_bBroken)
	{
		// Vector and dist between points
		glm::vec3 pointVector = m_pPoint2->GetPosition() - m_pPoint1->GetPosition(); 
		float pointDistance = glm::length(pointVector);

		// BREAK CONSTRAINT
		if (pointDistance > m_fRestDistance * 2.0f)
		{
			m_pPoint1->m_bOverExtended = true;
			m_pPoint2->m_bOverExtended = true;
		}

		glm::vec3 correctionVector = pointVector * (1 - m_fRestDistance / pointDistance);

		// Adjust for stiffness
		float correctionWeight1 = 0.5f * m_fStiffness;
		float correctionWeight2 = m_fStiffness - correctionWeight1;

		// Correct position of points
		m_pPoint1->ChangePos(correctionVector * correctionWeight1); 
		m_pPoint2->ChangePos(-correctionVector * correctionWeight2);
	}
}