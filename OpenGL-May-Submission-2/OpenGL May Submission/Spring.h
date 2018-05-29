#pragma once

#include "Point.h"

class Spring
{
public:

	Spring(Point* _point1, Point* _point2);

	void SatisfyConstraint();

private:
	float m_fRestDistance;
	Point* m_pPoint1;
	Point* m_pPoint2;
};