#include "Asteroid.h"


Asteroid::Asteroid()
{
	roll = 0;
	Vector4* verts = new Vector4[8];

	verts[0] = Vector4(-0.5f, 1,	-10,	1);
	verts[1] = Vector4(0.5f, 1,		-10,	1);
	verts[2] = Vector4(1, 0.5f,		-10,	1);
	verts[3] = Vector4(1, -0.5f,	-10,	1);
	verts[4] = Vector4(0.5f, -1,	-10,	1);
	verts[5] = Vector4(-0.5f, -1,	-10,	1);
	verts[6] = Vector4(-1, -0.5f,	-10,	1);
	verts[7] = Vector4(-1, 0.5f,	-10,	1);

	mesh = new Mesh();
	mesh->SetType(PRIMITIVE_LINE_LOOP);
	mesh->SetNumVerts(8);
	mesh->SetVertices(verts);
	mesh->SetDefaultUVs();
	mesh->SetDefaultColors();
}

void Asteroid::Update()
{
	roll += 0.1f;
	modelMatrix = Matrix4::Rotation(roll, Vector3(0, 0, 1));
}


Asteroid::~Asteroid()
{
}
