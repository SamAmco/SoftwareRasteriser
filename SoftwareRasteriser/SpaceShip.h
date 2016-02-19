#include "RenderObject.h"

#pragma once
class SpaceShip : public RenderObject
{
public:
	SpaceShip();
	~SpaceShip();

	void Update();

protected:
	Vector3 translate;
	const float moveSpeed = 0.1f;
};

