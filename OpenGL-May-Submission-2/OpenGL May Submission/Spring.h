#pragma once

#include "Point.h"

class Spring
{
public:

	Spring(Point* _point1, Point* _point2, float _restingDistance);

	void SatisfyConstraint();
	Point* m_pPoint1;
	Point* m_pPoint2;
	bool m_bBroken = false;

private:
	float m_fRestDistance;
};