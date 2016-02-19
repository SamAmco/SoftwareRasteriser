#include "SpaceShip.h"

SpaceShip::SpaceShip()
{
	translate = Vector3(0, -1, 0);
	Vector4* verts = new Vector4[6];

	verts[0] = Vector4(0,	0,		0,		1);
	verts[1] = Vector4(-1,	-0.5f,	-1,		1);
	verts[2] = Vector4(-1,	-0.5f,	0.5f,	1);
	verts[3] = Vector4(0,	0,		1,		1);
	verts[4] = Vector4(1,	-0.5f,	0.5f,	1);
	verts[5] = Vector4(1,	-0.5f,	-1,		1);

	mesh = new Mesh();
	mesh->SetType(PRIMITIVE_TRIANGLE_FAN);
	mesh->SetNumVerts(6);
	mesh->SetVertices(verts);
	mesh->SetDefaultUVs();
	mesh->SetDefaultColors();
}

void SpaceShip::Update()
{
	if (Keyboard::KeyDown(KEY_UP))
		translate += Vector3(0, 0, -moveSpeed);
	if (Keyboard::KeyDown(KEY_DOWN))
		translate += Vector3(0, 0, moveSpeed);
	if (Keyboard::KeyDown(KEY_LEFT))
		translate += Vector3(-moveSpeed, 0, 0);
	if (Keyboard::KeyDown(KEY_RIGHT))
		translate += Vector3(moveSpeed, 0, 0);

	modelMatrix = Matrix4::Translation(translate);
}

SpaceShip::~SpaceShip()
{
}
