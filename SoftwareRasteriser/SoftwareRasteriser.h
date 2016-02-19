/******************************************************************************
Class:SoftwareRasteriser
Implements:Window
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description: Class to encapsulate the various rasterisation techniques looked
at in the course material.

This is the class you'll be modifying the most!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Matrix4.h"
#include "Mesh.h"
#include "Texture.h"
#include "RenderObject.h"
#include "Common.h"
#include "Window.h"

#include <vector>

using std::vector;

struct BoundingBox 
{
	Vector2 topLeft;
	Vector2 bottomRight;
};

class RenderObject;
class Texture;

class SoftwareRasteriser : public Window	
{
public:
	SoftwareRasteriser(uint width, uint height);
	~SoftwareRasteriser(void);

	void	DrawObject(RenderObject*o);

	void	ClearBuffers();
	void	SwapBuffers();

	void	SetViewMatrix(const Matrix4 &m) 
	{
		viewMatrix		= m;
		viewProjMatrix	= projectionMatrix * viewMatrix;
	}
	
	void	SetProjectionMatrix(const Matrix4 &m) 
	{
		projectionMatrix	= m;
		viewProjMatrix		= projectionMatrix * viewMatrix;
	}

	static float ScreenAreaOfTri(const Vector4 & v0, const Vector4 & v1, const Vector4 & v2);


protected:
	Colour*	GetCurrentBuffer();

	void	RasterisePointsMesh(RenderObject*o);
	void	RasteriseLinesMesh(RenderObject*o);
	void	RasteriseLineLoopMesh(RenderObject*o);
	void	RasteriseTriMesh(RenderObject * o);
	void	RasteriseTriStripMesh(RenderObject *o);
	void	RasteriseTriFanMesh(RenderObject *o);

	virtual void Resize();

	void	RasteriseLine(const Vector4 &v0, const Vector4 &v1, 
		const Colour &colA = Colour(255,255,255,255), const Colour &colB = Colour(255,255,255,255), 
		const Vector3 &texA = Vector3() , const Vector3 &texB = Vector3());

	void RasteriseTri(
		const Vector4 & v0, const Vector4 & v1, const Vector4 & v2,
		const Colour & c0 = Colour(), const Colour & c1 = Colour(),
		const Colour & c2 = Colour(), const Vector3 & t0 = Vector3(),
		const Vector3 & t1 = Vector3(), const Vector3 & t2 = Vector3());

	BoundingBox CalculateBoxForTri(const Vector4 &a, const Vector4 &b, const Vector4 &c);

	
	bool CohenSutherlandLine(Vector4 & inA, Vector4 & inB,
							Colour & colA, Colour & colB,
							Vector3 & texA, Vector3 & texB);

	void SutherlandHodgmanTri(Vector4 & v0, Vector4 & v1, Vector4 & v2,
							const Colour & c0 = Colour(),
							const Colour & c1 = Colour(),
							const Colour & c2 = Colour(),
							const Vector2 & t0 = Vector2(),
							const Vector2 & t1 = Vector2(),
							const Vector2 & t2 = Vector2());

	float ClipEdge(const Vector4 & inA, const Vector4 & inB, int axis);
	int HomogeneousOutcode(const Vector4 & in);


	inline void	ShadePixel(uint x, uint y, const Colour&c) 
	{
		if(y >= screenHeight) {
			return;
		}
		if(x >= screenWidth) {
			return;
		}

		int index =  (y * screenWidth) + x;

		buffers[currentDrawBuffer][index] = c;
	}

	unsigned short*	depthBuffer;

	inline bool DepthFunc(int x, int y, float depthValue) 
	{
		int index = (y * screenWidth) + x;
		if (index < 0){
			return false; //TODO: make sure i am appropriate
		}

		unsigned int castVal = (unsigned int)depthValue;

		if (castVal > depthBuffer[index])
			return false;

		depthBuffer[index] = castVal;
		return true;
	}

	
	int		currentDrawBuffer;

	Colour*	buffers[2];

	

	Matrix4 viewMatrix;
	Matrix4 projectionMatrix;
	Matrix4 textureMatrix;

	Matrix4	viewProjMatrix;

	Matrix4	portMatrix;
};

