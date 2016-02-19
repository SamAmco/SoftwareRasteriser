#include "RenderObject.h"

#pragma once
class StarField : public RenderObject
{
public:

	StarField(uint numOfPoints);
	~StarField();


protected:

	float RandomFloat(float a, float b);
};

