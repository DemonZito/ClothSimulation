#pragma once

#include "Point.h"

class Spring
{
public:

	Spring(Point* _point1, Point* _point2, float _restingDistance, float _stiffness);

	void SatisfyConstraint();
	Point* m_pPoint1;
	Point* m_pPoint2;
	bool m_bBroken = false;
	float m_fStiffness = 1.0f;

private:
	float m_fRestDistance;
};