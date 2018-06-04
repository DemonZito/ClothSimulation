#include "Spring.h"

Spring::Spring(Point * _point1, Point * _point2)
	:
	m_pPoint1(_point1),
	m_pPoint2(_point2)
{
	glm::vec3 vector = _point1->GetPosition() - _point2->GetPosition();
	m_fRestDistance = glm::length(vector);
}

void Spring::SatisfyConstraint()
{
	if (!m_bBroken)
	{
		glm::vec3 pointVector = m_pPoint2->GetPosition() - m_pPoint1->GetPosition(); // vector from p1 to p2
		float pointDistance = glm::length(pointVector);


		//if (pointDistance > m_fRestDistance * 100.0f)
		//{
		//	m_pPoint1->RemoveLink(m_pPoint2);
		//	m_pPoint1->SetDetached(true);
		//	m_pPoint2->RemoveLink(m_pPoint1);
		//	//m_pPoint2->SetDetached(true);

		//	m_bBroken = true;
		//}

		glm::vec3 correctionVector = pointVector * (1 - m_fRestDistance / pointDistance);
		glm::vec3 correctionVectorHalf = correctionVector * 0.5f; // Lets make it half that length, so that we can move BOTH p1 and p2.
		m_pPoint1->ChangePos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		m_pPoint2->ChangePos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.
	}
	else
	{
		int i = 0;
	}
}