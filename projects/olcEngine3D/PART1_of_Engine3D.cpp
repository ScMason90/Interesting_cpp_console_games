/*PART 1;	finished during 10.17-10.23, Review during 10.25-10.26*/

#include "olcConsoleGameEngine.h"
using namespace std;

struct vec3d {
	float x, y, z;
};

struct triangle{
	vec3d p[3];
};

struct mesh {
	vector<triangle> tris;
};

struct mat4x4 {
	float m[4][4] = { 0.0f };
};

class olcEngine3D :public olcConsoleGameEngine {
public:
	olcEngine3D() {
		m_sAppName = L"3D demo";
	}

private:
	mesh meshCube;
	mat4x4 matProj;

	float fTheta;

	void MultiplyMatrixVector(vec3d& i, vec3d& o, mat4x4& m) {
		/*Notes that in javidx9 ways, the input/output 3D vector's form is a row,
		Accordingly MultiplyMatrix should be linear designed as column algebra first, 
		each col of formula corresponds a coordinate one by one from left to right*/
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f) {
			o.x /= w; o.y /= w; o.z /= w;
		}
	}

public:
	bool OnUserCreate() override {

		meshCube.tris = {
			// SOUTH
			{0.0f,0.0f,0.0f,	0.0f,1.0f,0.0f,		1.0f,1.0f,0.0f},
			{0.0f,0.0f,0.0f,	1.0f,1.0f,0.0f,		1.0f,0.0f,0.0f},

			// EAST
			{1.0f,0.0f,0.0f,	1.0f,1.0f,0.0f,		1.0f,1.0f,1.0f},
			{1.0f,0.0f,0.0f,	1.0f,1.0f,1.0f,		1.0f,0.0f,1.0f},

			// NORTH
			{1.0f,0.0f,1.0f,	1.0f,1.0f,1.0f,		0.0f,1.0f,1.0f},
			{1.0f,0.0f,1.0f,	0.0f,1.0f,1.0f,		0.0f,0.0f,1.0f},

			// WEST
			{0.0f,0.0f,1.0f,	0.0f,1.0f,1.0f,		0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f,	0.0f,1.0f,0.0f,		0.0f,0.0f,0.0f},

			// TOP
			{0.0f,1.0f,0.0f,	0.0f,1.0f,1.0f,		1.0f,1.0f,1.0f},
			{0.0f,1.0f,0.0f,	1.0f,1.0f,1.0f,		1.0f,1.0f,0.0f},

			// BOTTOM
			{1.0f,0.0f,1.0f,	0.0f,0.0f,1.0f,		0.0f,0.0f,0.0f},
			{1.0f,0.0f,1.0f,	0.0f,0.0f,0.0f,		1.0f,0.0f,0.0f}
		};

		// Projection Matrix
		float fNear = 0.1f;
		float fFar = 1000.0f;
		float fFov = 90.0f; // Degrees = Radian * (180 / PI);
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fFovRad = 1.0f / tanf(fFov * 0.5f * 3.14159f / 180.0f);

		// x'
		matProj.m[0][0] = fAspectRatio * fFovRad;
		// y'
		matProj.m[1][1] = fFovRad;
		// z'
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = -(fNear * fFar) / (fFar - fNear);
		// w'
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)override {

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		mat4x4 matRotX, matRotZ;
		fTheta += 1.0f * fElapsedTime;

		// Rotation Z
		// x'
		matRotZ.m[0][0] = cos(fTheta);
		matRotZ.m[1][0] = -sin(fTheta);
		// y'
		matRotZ.m[0][1] = sin(fTheta);
		matRotZ.m[1][1] = cos(fTheta);
		// Z & Homogeneous coordinate stay the same
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;


		// Rotation X ;rotate y&z plane
		// x 
		matRotX.m[0][0] = 1;
		// y'
		matRotX.m[1][1] = cos(fTheta);
		matRotX.m[2][1] = -sin(fTheta);
		// z'
		matRotX.m[1][2] = sin(fTheta);
		matRotX.m[2][2] = cos(fTheta);
		// homogeneous coordinate
		matRotX.m[3][3] = 1;


		// Draw Triangles
		for (auto tri : meshCube.tris) {
			triangle triProjected,triTranslated,triRotatedZ,triRotatedZX;

			// Z&X coordinates Rotation
			MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

			MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

			// Z coordinate Translation; inorder to have perspective visualization
			triTranslated = triRotatedZX;
			triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
			triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
			triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

			// Projection
			MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
			MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
			MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

			// Scale into view (coordinates of x&y,z_NDC already in scope[0,1])
			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

			triProjected.p[0].x *= 0.5f * ScreenWidth();
			triProjected.p[0].y *= 0.5f * ScreenHeight();
			triProjected.p[1].x *= 0.5f * ScreenWidth();
			triProjected.p[1].y *= 0.5f * ScreenHeight();
			triProjected.p[2].x *= 0.5f * ScreenWidth();
			triProjected.p[2].y *= 0.5f * ScreenHeight();

			DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
				triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y,
				PIXEL_SOLID, FG_WHITE);
		}

		return true;
	}
};

int main(void) {
	olcEngine3D demo;
	if (demo.ConstructConsole(184, 115, 5, 5)) {
		demo.Start();
	}

	return 0;
}