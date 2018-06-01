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
	const float stiffness = 1.0f;
	glm::vec3 pointVector = m_pPoint2->GetPosition() - m_pPoint1->GetPosition(); // vector from p1 to p2
	float pointDistance = glm::length(pointVector);

	float diff = (pointDistance - m_fRestDistance) / pointDistance;
	float Im1 = 1 / m_pPoint1->GetMass();
	float Im2 = 1 / m_pPoint2->GetMass();
	m_pPoint1->ChangePos((pointVector * (Im1 / (Im1 + Im2)) * stiffness * diff));
	m_pPoint2->ChangePos(-(pointVector * (Im2 / (Im1 + Im2)) * stiffness * diff));
}


/*
const float stiffness = 0.8f;
glm::vec3 pointVector = m_pPoint2->GetPosition() - m_pPoint1->GetPosition(); // vector from p1 to p2
float pointDistance = glm::length(pointVector);

float difference = (pointDistance - m_fRestDistance) / pointDistance;
float Im1 = 1 / m_pPoint1->GetMass();
float Im2 = 1 / m_pPoint2->GetMass();
m_pPoint1->ChangePos(-(pointVector * (Im1 / (Im1 + Im2)) * stiffness * difference));
m_pPoint2->ChangePos(pointVector * (Im2 / (Im1 + Im2)) * stiffness * difference);
*/

/*

const float stiffness = 0.8f;
glm::vec3 pointVector = m_pPoint2->GetPosition() - m_pPoint1->GetPosition(); // vector from p1 to p2
float pointDistance = glm::length(pointVector);

glm::vec3 correctionVector = pointVector * (1 - m_fRestDistance / pointDistance);
glm::vec3 correctionVectorHalf = correctionVector*0.5f; // Lets make it half that length, so that we can move BOTH p1 and p2.
m_pPoint1->ChangePos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
m_pPoint2->ChangePos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.
*/