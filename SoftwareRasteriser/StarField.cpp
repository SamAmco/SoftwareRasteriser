#include "StarField.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


StarField::StarField(uint numOfPoints)
{
	Vector4* verts = new Vector4[numOfPoints];

	for (uint i = 0; i < numOfPoints; ++i)
	{
		verts[i] = Vector4(RandomFloat(-30, 30),
			RandomFloat(-30, 30),
			RandomFloat(-30, 30),
			1);
	} 

	mesh = new Mesh();
	mesh->SetType(PRIMITIVE_POINTS);
	mesh->SetVertices(verts);
	mesh->SetNumVerts(numOfPoints);
}

float StarField::RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

StarField::~StarField()
{
}
