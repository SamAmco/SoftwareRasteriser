#include "SoftwareRasteriser.h"
#include <cmath>
#include <math.h>
#include <iostream>
/*
While less 'neat' than just doing a 'new', like in the tutorials, it's usually
possible to render a bit quicker to use direct pointers to the drawing area
that the OS gives you. For a bit of a speedup, you can uncomment the define below
to switch to using this method.

For those of you new to the preprocessor, here's a quick explanation:

Preprocessor definitions like #define allow parts of a file to be selectively enabled
or disabled at compile time. This is useful for hiding parts of the codebase on a
per-platform basis: if you have support for linux and windows in your codebase, obviously
the linux platform won't have the windows platform headers available, so compilation will
fail. So instead you can hide away all the platform specific stuff:

#if PLATFORM_WINDOWS
 DoSomeWindowsStuff();
#elif PLATFORM_LINUX
 DoSomeLinuxStuff();
 #else
 #error Unsupported Platform Specified!
 #endif

 As in our usage, it also allows you to selectively compile in some different functionality
 without any 'run time' cost - if it's not enabled by the preprocessor, it won't make it to
 the compiler, so no assembly will be generated.

Also, I've implemented the Resize method for you, in a manner that'll behave itself
no matter which method you use. I kinda forgot to do that, so there was a chance you'd
get exceptions if you resized to a bigger screen area. Sorry about that.
*/
//#define USE_OS_BUFFERS

SoftwareRasteriser::SoftwareRasteriser(uint width, uint height)	: Window(width, height)
{
	currentDrawBuffer	= 0;

#ifndef USE_OS_BUFFERS
	//Hi! In the tutorials, it's mentioned that we need to form our front + back buffer like so:
	for (int i = 0; i < 2; ++i) 
	{
		buffers[i] = new Colour[screenWidth * screenHeight];
	}
#else
	//This works, but we can actually save a memcopy by rendering directly into the memory the 
	//windowing system gives us, which I've added to the Window class as the 'bufferData' pointers
	for (int i = 0; i < 2; ++i) 
	{
		buffers[i] = (Colour*)bufferData[i];
	}
#endif

	depthBuffer		=	new unsigned short[screenWidth * screenHeight];

	float zScale	= (pow(2.0f,16) - 1) * 0.5f;

	Vector3 halfScreen = Vector3((screenWidth - 1) * 0.5f, (screenHeight - 1) * 0.5f, zScale);

	portMatrix = Matrix4::Translation(halfScreen) * Matrix4::Scale(halfScreen);
}

SoftwareRasteriser::~SoftwareRasteriser(void)	
{
#ifndef USE_OS_BUFFERS
	for(int i = 0; i < 2; ++i) {
		delete[] buffers[i];
	}
#endif
	delete[] depthBuffer;
}

void SoftwareRasteriser::Resize() 
{
	Window::Resize(); //make sure our base class gets to do anything it needs to

#ifndef USE_OS_BUFFERS
	for (int i = 0; i < 2; ++i) 
	{
		delete[] buffers[i];
		buffers[i] = new Colour[screenWidth * screenHeight];
	}
#else
	for (int i = 0; i < 2; ++i) 
	{
		buffers[i] = (Colour*)bufferData[i];
	}
#endif

	delete[] depthBuffer;
	depthBuffer = new unsigned short[screenWidth * screenHeight];

	float zScale = (pow(2.0f, 16) - 1) * 0.5f;

	Vector3 halfScreen = Vector3((screenWidth - 1) * 0.5f, (screenHeight - 1) * 0.5f, zScale);

	portMatrix = Matrix4::Translation(halfScreen) * Matrix4::Scale(halfScreen);
}

Colour*	SoftwareRasteriser::GetCurrentBuffer() 
{
	return buffers[currentDrawBuffer];
}

void	SoftwareRasteriser::ClearBuffers() 
{
	Colour* buffer = GetCurrentBuffer();

	unsigned int clearVal = 0xFF000000;
	unsigned int depthVal = ~0;

	for(uint y = 0; y < screenHeight; ++y) {
		for(uint x = 0; x < screenWidth; ++x) {
			buffer[(y * screenWidth) + x].c  = clearVal;
			depthBuffer[(y * screenWidth) + x] = depthVal;
		}
	}
}

void	SoftwareRasteriser::SwapBuffers() 
{
	PresentBuffer(buffers[currentDrawBuffer]);
	currentDrawBuffer = !currentDrawBuffer;
}

void	SoftwareRasteriser::DrawObject(RenderObject*o) 
{
	switch (o->GetMesh()->GetType())
	{
		case PRIMITIVE_POINTS:
		{
			RasterisePointsMesh(o);
		} break;
		case PRIMITIVE_LINES: 
		{
			RasteriseLinesMesh(o);
		} break;
		case PRIMITIVE_LINE_LOOP:
		{
			RasteriseLineLoopMesh(o);
		}break;
		case PRIMITIVE_TRIANGLES: 
		{
			RasteriseTriMesh(o);
		} break;
		case PRIMITIVE_TRIANGLE_STRIP:
		{
			RasteriseTriStripMesh(o);
		}break;
		case PRIMITIVE_TRIANGLE_FAN:
		{
			RasteriseTriFanMesh(o);
		}break;
	}
}

void SoftwareRasteriser::RasteriseTriStripMesh(RenderObject *o)
{
	Matrix4 mvp = viewProjMatrix * o->GetModelMatrix();

	for (uint i = 2; i < o->GetMesh()->numVertices; i++)
	{
		int ind1 = i;
		int ind2 = (i % 2 == 0) ? (i - 1) : (i - 2);
		int ind3 = (i % 2 == 0) ? (i - 2) : (i - 1);

		Vector4 v0 = mvp * o->GetMesh()->vertices[ind1];
		Vector4 v1 = mvp * o->GetMesh()->vertices[ind2];
		Vector4 v2 = mvp * o->GetMesh()->vertices[ind3];

		SutherlandHodgmanTri(v0, v1, v2,
			o->GetMesh()->colours[ind1], o->GetMesh()->colours[ind2], o->GetMesh()->colours[ind3],
			o->GetMesh()->textureCoords[ind1], o->GetMesh()->textureCoords[ind2],
			o->GetMesh()->textureCoords[ind3]);
	}
}

void SoftwareRasteriser::RasteriseTriFanMesh(RenderObject *o)
{
	Matrix4 mvp = viewProjMatrix * o->GetModelMatrix();

	for (uint i = 2; i < o->GetMesh()->numVertices; i++)
	{
		Vector4 v0 = mvp * o->GetMesh()->vertices[0];
		Vector4 v1 = mvp * o->GetMesh()->vertices[i - 1];
		Vector4 v2 = mvp * o->GetMesh()->vertices[i];

		SutherlandHodgmanTri(v0, v1, v2,
			o->GetMesh()->colours[i], o->GetMesh()->colours[i + 1], o->GetMesh()->colours[i + 2],
			o->GetMesh()->textureCoords[i], o->GetMesh()->textureCoords[i + 1],
			o->GetMesh()->textureCoords[i + 2]);
	}
}

void	SoftwareRasteriser::RasteriseLineLoopMesh(RenderObject*o)
{
	Matrix4 mvp = viewProjMatrix * o->GetModelMatrix();

	uint n = (uint)o->GetMesh()->numVertices;
	for (uint i = 0; i < n; i++)
	{
		int ind1 = (i == 0) ? (n - 1) : (i - 1);
		int ind2 = i;

		Vector4 v0 = mvp * o->GetMesh()->vertices[ind1];
		Vector4 v1 = mvp * o->GetMesh()->vertices[ind2];

		Colour c0 = o->GetMesh()->colours[ind1];
		Colour c1 = o->GetMesh()->colours[ind2];

		Vector3 t0 = Vector3(
			o->GetMesh()->textureCoords[ind1].x,
			o->GetMesh()->textureCoords[ind1].y, 1.0f);

		Vector3 t1 = Vector3(
			o->GetMesh()->textureCoords[ind2].x,
			o->GetMesh()->textureCoords[ind2].y, 1.0f);

		if (!CohenSutherlandLine(v0, v1, c0, c1, t0, t1))
			continue;

		t0.z = 1.0f;
		t1.z = 1.0f;

		t0 /= v0.w;
		t1 /= v1.w;

		v0.SelfDivisionByW();
		v1.SelfDivisionByW();

		RasteriseLine(v0, v1, c0, c1, t0, t1);
	}
}

void	SoftwareRasteriser::RasterisePointsMesh(RenderObject*o) 
{
	Matrix4 mvp = viewProjMatrix * o -> GetModelMatrix();

	for (uint i = 0; i < o -> GetMesh() -> numVertices; ++i)
	{
		Vector4 vertexPos = mvp * o -> GetMesh() -> vertices[i];
		vertexPos.SelfDivisionByW();

		Vector4 screenPos = portMatrix * vertexPos;
		//std::cout << "DrawingPoint: " << i << "\n";
		ShadePixel((uint)screenPos.x, (uint)screenPos.y, Colour::White);
	}
}

void	SoftwareRasteriser::RasteriseLinesMesh(RenderObject*o)
{
	Matrix4 mvp = viewProjMatrix * o->GetModelMatrix();

	for (uint i = 0; i < o->GetMesh()->numVertices; i += 2)
	{
		Vector4 v0 = mvp * o->GetMesh()->vertices[i];
		Vector4 v1 = mvp * o->GetMesh()->vertices[i + 1];

		Colour c0 = o->GetMesh()->colours[i];
		Colour c1 = o->GetMesh()->colours[i + 1];

		Vector3 t0 = Vector3(
			o->GetMesh()->textureCoords[i].x,
			o->GetMesh()->textureCoords[i].y, 1.0f);

		Vector3 t1 = Vector3(
			o->GetMesh()->textureCoords[i + 1].x,
			o->GetMesh()->textureCoords[i + 1].y, 1.0f);

		if (!CohenSutherlandLine(v0, v1, c0, c1, t0, t1))
			continue;

		t0.z = 1.0f;
		t1.z = 1.0f;

		t0 /= v0.w;
		t1 /= v1.w;

		v0.SelfDivisionByW();
		v1.SelfDivisionByW();

		RasteriseLine(v0, v1, c0, c1, t0, t1);
	}
}

bool SoftwareRasteriser::CohenSutherlandLine(Vector4 & inA, Vector4 & inB,
										Colour & colA, Colour & colB,
										Vector3 & texA, Vector3 & texB) 
{
	for (int i = 0; i < 6; ++i) 
	{
		int planeCode = 1 << i;
		int outsideA = (HomogeneousOutcode(inA) & planeCode);
		int outsideB = (HomogeneousOutcode(inB) & planeCode);
	
		if (outsideA && outsideB) 
		{
			return false; // both on wrong side of the plane , continue
		}
		if (!outsideA && !outsideB) 
		{
			continue; // both on side side of this plane , discard
		}
		else
		{
			float clipRatio = ClipEdge(inA, inB, planeCode);
			if (outsideA) 
			{
				texA = Vector3::Lerp(texA, texB, clipRatio);
				colA = Colour::Lerp(colA, colB, clipRatio);
				inA = Vector4::Lerp(inA, inB, clipRatio);
			}
			else 
			{
				texB = Vector3::Lerp(texA, texB, clipRatio);
				colB = Colour::Lerp(colA, colB, clipRatio);
				inB = Vector4::Lerp(inA, inB, clipRatio);
			}
		}
	}
	return true;
}


void	SoftwareRasteriser::RasteriseTriMesh(RenderObject* o)
{
	Matrix4 mvp = viewProjMatrix * o->GetModelMatrix();

	for (uint i = 0; i < o->GetMesh()->numVertices; i += 3)
	{
		Vector4 v0 = mvp * o->GetMesh()->vertices[i];
		Vector4 v1 = mvp * o->GetMesh()->vertices[i + 1];
		Vector4 v2 = mvp * o->GetMesh()->vertices[i + 2];

		SutherlandHodgmanTri(v0, v1, v2,
			o->GetMesh()->colours[i], o->GetMesh()->colours[i + 1], o->GetMesh()->colours[i + 2],
			o->GetMesh()->textureCoords[i], o->GetMesh()->textureCoords[i + 1],
			o->GetMesh()->textureCoords[i + 2]);
	}
}

const int INSIDE_CS =	0; // 000000
const int LEFT_CS =		1; // 000001
const int RIGHT_CS =	2; // 000010
const int BOTTOM_CS =	4; // 000100
const int TOP_CS =		8; // 001000
const int NEAR_CS =		16; // 010000
const int FAR_CS =		32; // 100000
int SoftwareRasteriser::HomogeneousOutcode(const Vector4 & in) 
{
	int outCode = INSIDE_CS;

	if (in.x < -in.w) 
		outCode |= LEFT_CS;
	else if (in.x > in.w) 
		outCode |= RIGHT_CS;

	if (in.y < -in.w) 
		outCode |= BOTTOM_CS;
	else if (in.y > in.w)
		outCode |= TOP_CS;

	if (in.z < -in.w)
		outCode |= NEAR_CS;
	else if (in.z > in.w)
		outCode |= FAR_CS;

	return outCode;
}

float SoftwareRasteriser::ClipEdge(const Vector4 & inA, const Vector4 & inB, int axis) 
{
	float ratio = 0.0f;

	switch (axis)
	{
		// ’Negative w’ planes subtract the w of a and add the w of b
		// ’Positive w’ planes add the w of a and subtract the w of b
		case LEFT_CS: ratio = (-inA.w - inA.x) /
			((inB.x - inA.x) + inB.w - inA.w); break;

		case RIGHT_CS: ratio = (inA.w - inA.x) /
			((inB.x - inA.x) - inB.w + inA.w); break;

		case BOTTOM_CS: ratio = (-inA.w - inA.y) /
			((inB.y - inA.y) + inB.w - inA.w); break;
		case TOP_CS: ratio = (inA.w - inA.y) /
			((inB.y - inA.y) - inB.w + inA.w); break;

		case NEAR_CS: ratio = (-inA.w - inA.z) /
			((inB.z - inA.z) + inB.w - inA.w); break;
		case FAR_CS: ratio = (inA.w - inA.z) /
			((inB.z - inA.z) - inB.w + inA.w); break;
	}
	return min(1.0f, ratio);
}


#define MAX_VERTS 15
void SoftwareRasteriser::SutherlandHodgmanTri(Vector4& v0, Vector4& v1, Vector4& v2,
	const Colour& c0, const Colour& c1, const Colour& c2,
	const Vector2& t0, const Vector2& t1, const Vector2& t2)
{
	Vector4 posIn[MAX_VERTS];
	Colour colIn[MAX_VERTS];
	Vector3 texIn[MAX_VERTS];

	Vector4 posOut[MAX_VERTS];
	Colour colOut[MAX_VERTS];
	Vector3 texOut[MAX_VERTS];

	posIn[0] = v0;
	posIn[1] = v1;
	posIn[2] = v2;

	colIn[0] = c0;
	colIn[1] = c1;
	colIn[2] = c2;

	texIn[0] = Vector3(t0.x, t0.y, 1);
	texIn[1] = Vector3(t1.x, t1.y, 1);
	texIn[2] = Vector3(t2.x, t2.y, 1);

	int inSize = 3;

	for (int i = 0; i <= 6; i++)
	{
		int planeCode = 1 << i;

		Vector4 prevPos = posIn[inSize - 1];
		Colour prevCol = colIn[inSize - 1];
		Vector3 prevTex = texIn[inSize - 1];

		int outSize = 0; // Keep track of the output list size ...

		for (int j = 0; j < inSize; ++j) 
		{
			int outsideA = (HomogeneousOutcode(posIn[j]) & planeCode);
			int outsideB = (HomogeneousOutcode(prevPos) & planeCode);

			if (outsideA ^ outsideB) 
			{
				float clipRatio = ClipEdge(posIn[j], prevPos, planeCode);

				colOut[outSize] = Colour::Lerp(colIn[j], prevCol, clipRatio);
				texOut[outSize] = Vector3::Lerp(texIn[j], prevTex, clipRatio);
				posOut[outSize] = Vector4::Lerp(posIn[j], prevPos, clipRatio);

				outSize++;
			}

			if (!outsideA) 
			{
				posOut[outSize] = posIn[j];
				colOut[outSize] = colIn[j];
				texOut[outSize] = texIn[j];
				outSize++;
			}

			prevPos = posIn[j];
			prevCol = colIn[j];
			prevTex = texIn[j];

		}// End of vertex processing loop

		for (int j = 0; j < outSize; ++j) 
		{
			posIn[j] = posOut[j];
			colIn[j] = colOut[j];
			texIn[j] = texOut[j];
		}
		inSize = outSize;

	}// End of plane clipping loop

	for (int i = 0; i < inSize; ++i) 
	{
		texIn[i] = Vector3(texIn[i].x, texIn[i].y, 1.0f) / posIn[i].w;
		posIn[i].SelfDivisionByW();
	}
	for (int i = 2; i < inSize; ++i) 
	{
		RasteriseTri(
			posIn[0], posIn[i - 1], posIn[i],
			colIn[0], colIn[i - 1], colIn[i],
			texIn[0], texIn[i - 1], texIn[i]);
	}
}


BoundingBox SoftwareRasteriser::CalculateBoxForTri(
	const Vector4 &a, const Vector4 &b, const Vector4 &c)
{
	BoundingBox box;
	
	box.topLeft.x = a.x; // Start with the first vertex value
	box.topLeft.x = min(box.topLeft.x, b.x); // swap to second if less
	box.topLeft.x = min(box.topLeft.x, c.x); // swap to third if less
	box.topLeft.x = max(box.topLeft.x, 0.0f); // Screen Bound
	
	box.topLeft.y = a.y;
	box.topLeft.y = min(box.topLeft.y, b.y);
	box.topLeft.y = min(box.topLeft.y, c.y);
	box.topLeft.y = max(box.topLeft.y, 0.0f); // Screen Bound
	
	box.bottomRight.x = a.x; // Start with the first vertex value
	box.bottomRight.x = max(box.bottomRight.x, b.x); // swap if more
	box.bottomRight.x = max(box.bottomRight.x, c.x); // swap if more
	box.bottomRight.x = min(box.bottomRight.x, screenWidth); // Bound
	
	box.bottomRight.y = a.y;
	box.bottomRight.y = max(box.bottomRight.y, b.y);
	box.bottomRight.y = max(box.bottomRight.y, c.y);
	box.bottomRight.y = min(box.bottomRight.y, screenHeight); // Bound
	
	return box;
}

float SoftwareRasteriser::ScreenAreaOfTri(
	const Vector4 &a, const Vector4 &b, const Vector4 & c) 
{
	float area = ((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) - //1st diagonals
				 ((b.x * a.y) + (c.x * b.y) + (a.x * c.y)); //2nd diagonals
	return area * 0.5f;
}

void SoftwareRasteriser::RasteriseTri(
	const Vector4 & triA, const Vector4 & triB, const Vector4 & triC,
	const Colour & colA, const Colour & colB, const Colour & colC,
	const Vector3 & texA, const Vector3 & texB, const Vector3 & texC) 
{
	// Incoming triangles are in NDC space
	Vector4 v0 = portMatrix * triA;// Now in viewport space !
	Vector4 v1 = portMatrix * triB;// Now in viewport space !
	Vector4 v2 = portMatrix * triC;// Now in viewport space !

	 BoundingBox b = CalculateBoxForTri(v0, v1, v2);
	  
	 float triArea = ScreenAreaOfTri(v0, v1, v2);
	 if (triArea < 0.0f)
		 return;

	 float areaRecip = 1.0f / triArea;

	 float subTriArea[3];
	 Vector4 screenPos(0, 0, 0, 1);

	 for (float y = b.topLeft.y; y < b.bottomRight.y; ++y)
	 {
		 for (float x = b.topLeft.x; x < b.bottomRight.x; ++x)
		 {
			 screenPos.x = x; // Create vertex ’p’
			 screenPos.y = y; // Create vertex ’p’

			 subTriArea[0] = abs(ScreenAreaOfTri(v0, screenPos, v1));
			 subTriArea[1] = abs(ScreenAreaOfTri(v1, screenPos, v2));
			 subTriArea[2] = abs(ScreenAreaOfTri(v2, screenPos, v0));

			 float triSum = subTriArea[0] + subTriArea[1] + subTriArea[2];

			if (triSum > (triArea + 0.1f))
				 continue;


			if (triSum < 1.0f)  // Tiny triangle we don ’t care about ...
				 continue;

			float alpha = subTriArea[1] * areaRecip;
			float beta = subTriArea[2] * areaRecip;
			float gamma = subTriArea[0] * areaRecip;
			 
			float zVal = (v0.z * alpha) + (v1.z * beta) + (v2.z * gamma);

			if (!DepthFunc((int)x, (int)y, zVal)) 
				continue;

			Colour subColour = ((colA * alpha) +
								(colB * beta) +
								(colC * gamma));
			
			ShadePixel((uint)x, (uint)y, subColour);
		 }
	 }
}


void	SoftwareRasteriser::RasteriseLine(	const Vector4 & vertA, const Vector4 & vertB,
											const Colour & colA, const Colour & colB,
											const Vector3 & texA, const Vector3 & texB	)
{
	// Transform our ndc coordinates to screen coordinates
	Vector4 v0 = portMatrix * vertA;
	Vector4 v1 = portMatrix * vertB;
	Vector4 dir = v1 - v0;// What direction is the line going ?
	
	int xDir = (dir.x < 0.0f) ? -1 : 1; // move left or right ?
	int yDir = (dir.y < 0.0f) ? -1 : 1; // move up or down ?
	
	int x = (int)v0.x; // Current x axis plot point
	int y = (int)v0.y; // Current y axis plot point

	int *target = NULL;// Increment on error overflow , x or y?
	int *scan = NULL;// Increment every iteration , x or y?
	int scanVal = 0;
	int targetVal = 0;
	
	float slope = 0.0f;
	
	int range = 0; // Bound our loop by this value ;

	// if we are steep , we have to scan over the y axis
	if (abs(dir.y) > abs(dir.x)) 
	{
		slope = (dir.x / dir.y);
		range = (int)abs(dir.y);
		
		target = &x; // Error on x
		scan = &y; // Iterate over y
		scanVal = yDir;
		targetVal = xDir;
	}
	else 
	{ // not steep ? scan over the x axis ;
		slope = (dir.y / dir.x);
		range = (int)abs(dir.x);
		
		target = &y; // Error on y
		scan = &x; // Iterate over x
		scanVal = xDir;
		targetVal = yDir;
	}


	float absSlope = abs(slope);
	float error = 0.0f;
	
	float reciprocalRange = 1.0f / range;

	for (int i = 0; i < range; ++i)
	{
		float t = i * reciprocalRange;
		Colour currentCol = colB * t + colA *(1.0f - t);

		float zVal = (v1.z*t) + (v0.z)*(1.0f - t);// float zVal = v1.z *(i * reciprocalRange) + v0.z *(1.0f - (i * reciprocalRange));

		//if (DepthFunc((int)x, (int)y, zVal)) 
			ShadePixel(x, y, currentCol);

		error += absSlope;

		if (error > 0.5f)
		{
			error -= 1.0f;
			(*target) += targetVal; // Either advance along x or y
		}
		(*scan) += scanVal; // Either iterate along x or y
	}
}