#include "SoftwareRasteriser.h"

#include "Mesh.h"
#include "Texture.h"
#include "StarField.h"
#include "Asteroid.h"
#include "SpaceShip.h"

int main() 
{
	SoftwareRasteriser r(800,600);
	r.SetProjectionMatrix(Matrix4::Perspective(1.0f, 100.0f, 800.0f / 600.0f, 45.0f));

	RenderObject *triangle = new RenderObject();
	triangle->mesh = Mesh::GenerateTriangle();
	triangle->modelMatrix = Matrix4::Translation(Vector3(2, 0, -5));

	RenderObject *cube = new RenderObject();
	cube->mesh = Mesh::LoadMeshFile("cube.mesh");
	cube->modelMatrix = Matrix4::Translation(Vector3(-2, 0, -5));

	RenderObject *lines = new RenderObject();
	lines->mesh = Mesh::GenerateLines();
	lines->modelMatrix = Matrix4::Translation(Vector3(1.1f, 1.1f, -10));

	//These features I have implemented as their own objects because they have some
	//extra functionality above and beyond that of a mesh.
	StarField *starField = new StarField(1000);
	Asteroid *asteroid = new Asteroid();
	SpaceShip *spaceShip = new SpaceShip();//The space ship starts below the camera, YOU MUST USE THE ARROW KEYS TO MOVE IT INTO VIEW

	RenderObject *triangleStrip = new RenderObject();
	triangleStrip->mesh = Mesh::GenerateTriangleStrip(20);
	triangleStrip->modelMatrix = Matrix4::Translation(Vector3(0, 0, -20));

	float yaw = 0.0f;
	Vector3 move = Vector3();
	while (r.UpdateWindow() && !Keyboard::KeyTriggered(KEY_ESCAPE))
	{
		r.ClearBuffers();

		Matrix4 viewMatrix;

		yaw += Mouse::GetRelativePosition().x;

		if (Keyboard::KeyDown(KEY_A))
		{
			move += Vector3(0.01f, 0, 0);
		}
		if (Keyboard::KeyDown(KEY_D))
		{
			move += Vector3(-0.01f, 0, 0);
		}
		if (Keyboard::KeyDown(KEY_W))
		{
			move += Vector3(0, 0, 0.01f);
		}
		if (Keyboard::KeyDown(KEY_S))
		{
			move += Vector3(0, 0, -0.01f);
		}

		viewMatrix = viewMatrix * Matrix4::Rotation(yaw, Vector3(0, 1, 0));
		viewMatrix = viewMatrix * Matrix4::Translation(move);

		r.SetViewMatrix(viewMatrix);

		//Update functions here
		asteroid->Update();
		spaceShip->Update();

		//Put draw functions here!
		r.DrawObject(starField);
		r.DrawObject(triangle);
		r.DrawObject(cube);
		r.DrawObject(spaceShip);
		r.DrawObject(triangleStrip);
		r.DrawObject(lines);
		r.DrawObject(asteroid);
		r.SwapBuffers();
	}

	delete triangle;
	delete cube;
	delete asteroid;
	delete starField;
	delete spaceShip;
	delete triangleStrip;
	delete lines;

	return 0;
}