#include "Mesh.h"

Mesh::Mesh(void)	
{
	type			= PRIMITIVE_POINTS;

	numVertices		= 0;

	vertices		= NULL;
	colours			= NULL;
	textureCoords	= NULL;
}

Mesh::~Mesh(void)	
{
	delete[] vertices;
	delete[] colours;
	delete[] textureCoords;
}

Mesh* Mesh::LoadMeshFile(const string & filename) 
{
	ifstream f(filename);

	if (!f) 
	{
		return NULL;
	}

	Mesh *m = new Mesh();
	m -> type = PRIMITIVE_TRIANGLES;
	f >> m -> numVertices;

	int hasTex = 0;
	int hasColour = 0;

	f >> hasTex; // Oooh ! Sneak preview of later tutorial ...
	f >> hasColour;

	m -> vertices = new Vector4[m -> numVertices];
	m -> textureCoords = new Vector2[m -> numVertices];
	m -> colours = new Colour[m -> numVertices];

	for (uint i = 0; i < m -> numVertices; ++i) 
	{
		f >> m -> vertices[i].x;
		f >> m -> vertices[i].y;
		f >> m -> vertices[i].z;
	}
	if (hasColour) 
	{
		for (uint i = 0; i < m -> numVertices; ++i) 
		{
			f >> m -> colours[i].r; f >> m -> colours[i].g;
			f >> m -> colours[i].b; f >> m -> colours[i].a;
		}
	}
	return m;
}

void Mesh::SetVertices(Vector4* verts)
{
	delete[] vertices;

	vertices = verts;
}

void Mesh::SetNumVerts(uint num)
{
	numVertices = num;
}

void Mesh::SetType(PrimitiveType t)
{
	type = t;
}

void Mesh::SetDefaultColors()
{
	colours = new Colour[numVertices];
	for (uint i = 0; i < numVertices; i++)
	{
		colours[i] = Colour(255, 255, 255, 255);
	}
}

void Mesh::SetDefaultUVs()
{
	textureCoords = new Vector2[numVertices];
	for (uint i = 0; i < numVertices; i++)
	{
		textureCoords[i] = Vector2(0, 0);
	}
}

Mesh* Mesh::GenerateLine(const Vector3 &from, const Vector3 &to)
{
	Mesh* m = new Mesh();
	
	m -> numVertices = 2;
	
	m -> vertices = new Vector4[m -> numVertices];
	m -> colours = new Colour[m -> numVertices];
	m->textureCoords = new Vector2[m->numVertices];

	m -> vertices[0] = Vector4(from.x, from.y, from.z, 1.0f);
	m -> vertices[1] = Vector4(to.x, to.y, to.z, 1.0f);

	m -> colours[0] = Colour(255, 0, 0, 255);
	m -> colours[1] = Colour(0, 0, 255, 255); 

	m->textureCoords[0] = Vector2();
	m->textureCoords[1] = Vector2();
	
	m -> type = PRIMITIVE_LINES;
	
	return m;
}

Mesh* Mesh::GenerateLines()
{
	Mesh* m = new Mesh();

	m->numVertices = 6;

	m->vertices = new Vector4[m->numVertices];
	m->colours = new Colour[m->numVertices];
	m->textureCoords = new Vector2[m->numVertices];

	m->vertices[0] = Vector4(1,		1,		0,		1);
	m->colours[0] = Colour(0,		0,		0,		255);
	m->vertices[1] = Vector4(0,		0,		0,		1);
	m->colours[1] = Colour(255,		0,		0,		255);

	m->vertices[2] = Vector4(0.1f,	0.2f,	0,		1);
	m->colours[2] = Colour(0,		0,		0,		255);
	m->vertices[3] = Vector4(0,		0,		0,		1);
	m->colours[3] = Colour(255,		0,		0,		255);

	m->vertices[4] = Vector4(0.2f,	0.1f,	0,		1);
	m->colours[4] = Colour(0,		0,		0,		255);
	m->vertices[5] = Vector4(0,		0,		0,		1);
	m->colours[5] = Colour(255,		0,		0,		255);


	for (uint i = 0; i < m->numVertices; i++)
	{
		m->textureCoords[i] = Vector2(0, 0);
	}

	m->type = PRIMITIVE_LINES;

	return m;
}

Mesh* Mesh::GenerateTriangle()
{
	Mesh * m = new Mesh();
	m -> type = PRIMITIVE_TRIANGLES;
	
	m -> numVertices = 3;
	
	m -> vertices = new Vector4[m -> numVertices];
	m -> colours = new Colour[m -> numVertices];
	m -> textureCoords = new Vector2[m -> numVertices];


	m -> vertices[0] = Vector4(0.5f, -0.5f, 0, 1.0f);
	m -> vertices[1] = Vector4(0.0f, 0.5f, 0, 1.0f);
	m -> vertices[2] = Vector4(-0.5f, -0.5f, 0, 1.0f);

	m -> colours[0] = Colour(255, 0, 0, 255);
	m -> colours[1] = Colour(0, 255, 0, 255);
	m -> colours[2] = Colour(0, 0, 255, 255);

	m -> textureCoords[0] = Vector2(0, 0);
	m -> textureCoords[1] = Vector2(0, 0);
	m -> textureCoords[2] = Vector2(0, 0);
	
	return m;
}

Mesh* Mesh::GenerateTriangleStrip(int vertCount)
{
	Mesh* m = new Mesh();
	m->type = PRIMITIVE_TRIANGLE_STRIP;

	m->numVertices = vertCount;
	m->vertices = new Vector4[m->numVertices];
	m->colours = new Colour[m->numVertices];
	m->textureCoords = new Vector2[m->numVertices];

	float startX = -0.5f * (float)(vertCount / 2);

	for (int i = 0; i < vertCount; i++)
	{
		m->vertices[i] = Vector4(startX + (i * 0.5f), (i % 2 == 0) ? -0.5f : 0.5f, 0, 1);
		m->textureCoords[i] = Vector2(0, 0);
		switch (i % 3)
		{
		case 0:
			m->colours[i] = Colour(255, 0, 0, 255);
			break;
		case 1:
			m->colours[i] = Colour(0, 255, 0, 255);
			break;
		case 2:
			m->colours[i] = Colour(0, 0, 255, 255);
			break;
		}
	}

	return m;
}