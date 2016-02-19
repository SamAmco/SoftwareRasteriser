#include "RenderObject.h"

#pragma once
class Asteroid : public RenderObject
{
public:
	Asteroid();
	~Asteroid();

	void Update();

protected:
	float roll;
};

