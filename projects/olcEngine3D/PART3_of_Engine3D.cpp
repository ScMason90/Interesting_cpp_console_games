/*PART 3;	finished during 10.30-11.13, Review during 11.15-11.17.*/

#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;


struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};

struct triangle
{
	vec3d p[3];

	wchar_t sym;
	short col;
};

struct mesh
{
	vector<triangle> tris;

	bool LoadFromObjectFile(string sFilename)
	{
		ifstream f(sFilename);
		if (!f.is_open())
			return false;

		// Local cache of verts
		vector<vec3d> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

		return true;
	}

};

struct mat4x4
{
	float m[4][4] = { 0 };
};

class olcEngine3D : public olcConsoleGameEngine
{
public:
	olcEngine3D()
	{
		m_sAppName = L"3D Demo";
	}


private:
	mesh meshCube;
	mat4x4 matProj;

	vec3d vCamera;
	vec3d vLookDir;

	float fYaw;
	
	float fTheta;

	vec3d Matrix_MultiplyVector(vec3d& i, mat4x4& m)
	{
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		return v;
	}

	mat4x4 Matrix_MakeIdentity()
	{					// Matrix must be nozero diagonally in 3d transformation
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationX(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][1] = -sinf(fAngleRad);
		matrix.m[1][2] = sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationY(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[2][0] = -sinf(fAngleRad);
		matrix.m[1][1] = 1.0f;
		matrix.m[0][2] = sinf(fAngleRad);
		matrix.m[2][2] = cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotationZ(float fAngleRad)
	{
		mat4x4 matrix;
		matrix.m[0][0] = cosf(fAngleRad);
		matrix.m[1][0] = -sinf(fAngleRad);
		matrix.m[0][1] = sinf(fAngleRad);
		matrix.m[1][1] = cosf(fAngleRad);
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeTranslation(float x, float y, float z)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 3.14159f * 0.5f / 180.0f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][2] = -(fNear * fFar / (fFar - fNear));
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

	mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
	{				// Aim to aggregate all Matrix Transformations.
		mat4x4 matrix;
		for (int c = 0; c < 4; c++) {
			for (int r = 0; r < 4; r++) {
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
			}
		}
		return matrix;
	}

	mat4x4 Matrix_PointAt(vec3d& pos, vec3d& target, vec3d& up)
	{
		// Calculate new Forward direction
		vec3d newForward = Vector_Sub(target, pos);
		newForward = Vector_Normalise(newForward);
		// ¡ü Don't forget to normalise these new axis vectors

		// Calculate new Up direction
		vec3d a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
		vec3d newUp = Vector_Sub(up, a);
		newUp = Vector_Normalise(newUp);  //¡ú Still 'Dont forget...'

		// Right direction is easy, it's just cross product
		vec3d newRight = Vector_CrossProduct(newUp, newForward);
		/* ¡ü Params of cross product are already unit vectors,
		So no need normalise new Right*/

		/*Original note : Construct Dimensioning and Translation matrix
		Adjust_annotation:Construct camera-to-world matrix (Rotation and Translation)*/
		mat4x4 matrix;
		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_QuickInverse(mat4x4& m)	// Only work for Rotation and Translation Matrices
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0];	matrix.m[0][1] = m.m[1][0];	matrix.m[0][2] = m.m[2][0];	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1];	matrix.m[1][1] = m.m[1][1];	matrix.m[1][2] = m.m[2][1];	matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2];	matrix.m[2][1] = m.m[1][2];	matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * m.m[0][0] + m.m[3][1] * m.m[0][1] + m.m[3][2] * m.m[0][2]);
		matrix.m[3][1] = -(m.m[3][0] * m.m[1][0] + m.m[3][1] * m.m[1][1] + m.m[3][2] * m.m[1][2]);
		matrix.m[3][2] = -(m.m[3][0] * m.m[2][0] + m.m[3][1] * m.m[2][1] + m.m[3][2] * m.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	vec3d Vector_Add(vec3d& v1, vec3d& v2)
	{
		return { v1.x + v2.x,v1.y + v2.y,v1.z + v2.z };
	}

	vec3d Vector_Sub(vec3d& v1, vec3d& v2)
	{
		return { v1.x - v2.x,v1.y - v2.y,v1.z - v2.z };
	}

	vec3d Vector_Mul(vec3d& v1, float k)	// 3d Vector multiply a scalar
	{
		return { v1.x * k,v1.y * k,v1.z * k };
	}

	vec3d Vector_Div(vec3d& v1, float k)	// 3d Vector divide by a scalar
	{
		return { v1.x / k,v1.y / k,v1.z / k };
	}

	float Vector_DotProduct(vec3d& v1, vec3d& v2)	// Result is a scalar
	{
		return  v1.x * v2.x + v1.y* v2.y + v1.z* v2.z;
	}

	float Vector_Length(vec3d& v)
	{
		return sqrtf(Vector_DotProduct(v, v));
	}

	vec3d Vector_Normalise(vec3d& v)
	{
		float l = Vector_Length(v);
		return Vector_Div(v, l);	// {v.x/l,v.y/l,v.z/l};
	}

	vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2)
	{
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}

	vec3d Vector_IntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& LineStart, vec3d& LineEnd)
	{
		plane_n = Vector_Normalise(plane_n);
		float plane_d = -Vector_DotProduct(plane_p, plane_n);
		float ad = Vector_DotProduct(plane_n, LineStart);
		float bd = Vector_DotProduct(plane_n, LineEnd);
		float t = (-plane_d - ad) / (bd - ad);
		vec3d LineStartToEnd = Vector_Sub(LineEnd, LineStart);
		vec3d LineIntersect = Vector_Mul(LineStartToEnd, t);
		return Vector_Add(LineStart, LineIntersect);
	}

	int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& in_tri, triangle& out_tri1, triangle& out_tri2)
	{
		// Make sure plane normal is indeed normal
		plane_n = Vector_Normalise(plane_n);

		// Return signed shortest distance from point to plane, plane normal must be normalised
		auto dist = [&](vec3d& p)
			{
				vec3d PlanepToP = Vector_Sub(p, plane_p);
				return Vector_DotProduct(plane_n, PlanepToP);
			};

		/*Create two temporary storage arrays to classify points either side of plane
		If distance sign is positive, point lies on "inside" of plane*/
		vec3d* inside_points[3];	int nInsidePointCount = 0;
		vec3d* outside_points[3];	int nOutsidePointCount = 0;

		// Get signed distance of each point in triangle to plane
		float d0 = dist(in_tri.p[0]);
		float d1 = dist(in_tri.p[1]);
		float d2 = dist(in_tri.p[2]);

		if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[0]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[0]; }
		if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[1]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[1]; }
		if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.p[2]; }
		else { outside_points[nOutsidePointCount++] = &in_tri.p[2]; }

		/*Now classify triangle points, and break the input triangle into 
		smaller output triangles if required.There are four possible 
		outcomes....*/

		if (nOutsidePointCount == 3 && nInsidePointCount == 0)
		{
			/*All points lie on the outside of plane, so clip whole triangle
			It ceases to exist*/

			return 0;	// No returned triangle are valid
		}

		if (nInsidePointCount == 3 && nOutsidePointCount == 0)
		{	
			/*All points lie on the inside of plane, so do nothing
			and allow the triangle to simply pass through*/
			out_tri1 = in_tri;

			return 1;	// Just the one returned triangle is valid
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2)
		{
			/*Triangle should be clipped.As two points lie outside
			the plane.The triangle simply becomes a smaller triangle*/

			// Copy appearance info to new triangle
			out_tri1.col = in_tri.col;	//FG_BLUE;
			out_tri1.sym = in_tri.sym;

			// The inside point is valid, so keep that...
			out_tri1.p[0] = *inside_points[0];

			/*but the two new points are at the locations where the
			original sides of the triangle(lines) intersect with the plane*/
			out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

			return 1;	// Return the newly formed single triangle
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1)
		{
			/*Triangle should be clipped.As two points lie inside the plane.
			The triangle simply becomes a "quad".Fortunately, we can
			represent a "quad" with two triangles.*/

			// Copy apperearance to new triangles
			//out_tri1.col = FG_RED;	out_tri2.col = FG_GREEN;
			out_tri2.col = out_tri1.col = in_tri.col;
			out_tri1.sym = out_tri2.sym = in_tri.sym;

			/*The first triangle consists of two original points and a new
			point determined by the location where one side of the triangle
			intersect with the plane*/
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

			/*The second triangle is composed of one of the inside points, a
			new point determined by the intersection of the other side of the
			triangle and plane, and the newly created point above*/
			out_tri2.p[0] = out_tri1.p[2];
			out_tri2.p[1] = *inside_points[1];
			out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

			return 2;	// Return two newly formed triangles which form a "quad"
		}
	}



	// Taken From Command Line Webcam Video
	CHAR_INFO GetColour(float lum)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}

public:
	bool OnUserCreate() override
	{

		meshCube.LoadFromObjectFile("Source_Files/mountains.obj");

		// Projection Matrix
		matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{	
		if (GetKey(VK_UP).bHeld)
		{
			vCamera.y += 8.0f * fElapsedTime;
		}
		if (GetKey(VK_DOWN).bHeld)
		{
			vCamera.y -= 8.0f * fElapsedTime;
		}
		/*Conflict between standard Vector&Matrix libary under Left-Hand cross 
		product rule and OLCengine3D Easy coordinate render mode caused confusing.*/ 
		if (GetKey(VK_LEFT).bHeld)
		{
			vCamera.x += 8.0f * fElapsedTime;
		}
		if (GetKey(VK_RIGHT).bHeld)
		{
			vCamera.x -= 8.0f * fElapsedTime;
		}	// ¡û,¡ú movement snippet here is literally Lazy Optimization

		vec3d vForward = Vector_Mul(vLookDir, 8.0f * fElapsedTime);
		if (GetKey('W').bHeld)
		{
			vCamera = Vector_Add(vCamera, vForward);
		}
		if (GetKey('S').bHeld)
		{
			vCamera = Vector_Sub(vCamera, vForward);
		}
		/*Matrix_RotationY of Vectors-Matrix Libary is whithin Right-hand,So we
		need to invert RHed increasing rads (CCW¡úCW,CW¡úCCW) to keep sync with 
		basic axis in current LH cartesian space.*/
		if (GetKey('A').bHeld)
		{
			fYaw -= 2.0f * fElapsedTime;
		}
		if (GetKey('D').bHeld)
		{
			fYaw += 2.0f * fElapsedTime;
		}

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Set up rotation matrices
		mat4x4 matRotZ, matRotX;
		//fTheta += 1.0f * fElapsedTime;

		matRotZ = Matrix_MakeRotationZ(fTheta);	// Rotation Z
		matRotX = Matrix_MakeRotationX(fTheta * 0.5f);	// Rotation X

		mat4x4 matTrans;
		matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 4.0f);

		mat4x4 matWorld = Matrix_MakeIdentity();
		matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

		vec3d vTarget = { 0,0,1 };
		vec3d vUp = { 0,1,0 };
		mat4x4 matCamRotY = Matrix_MakeRotationY(fYaw);
		vLookDir = Matrix_MultiplyVector(vTarget, matCamRotY);
		vTarget = Vector_Add(vCamera, vLookDir);

		// Create camera-to-world Matrix
		mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		// Make View Matrix (Inverse;camera-to-world->world-to-camera)
		mat4x4 matView = Matrix_QuickInverse(matCamera);

		// Store triagles for rastering later
		vector<triangle> vecTrianglesToRaster;

		// Draw Triangles
		for (auto tri : meshCube.tris)
		{
			triangle triProjected, triTransformed, triViewed;

			// World Matrix Transformed
			triTransformed.p[0] = Matrix_MultiplyVector(tri.p[0], matWorld);
			triTransformed.p[1] = Matrix_MultiplyVector(tri.p[1], matWorld);
			triTransformed.p[2] = Matrix_MultiplyVector(tri.p[2], matWorld);

			// Calculate Triangle Normal
			vec3d normal, line1, line2;
			
			// Get line either side of triangle
			line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

			// Take cross product of lines to get normal to triangle surface
			normal = Vector_CrossProduct(line1, line2);

			// You normally need to normalise normal
			normal = Vector_Normalise(normal);

			/*Original note : Get ray from triangle to camera
			Self_Rectified : Get ray from camera to triangle*/ 
			vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

			/*Original note : If ray is aligned with normal, then triangle is visible
			Self_Rectified : If camera ray and normal from an obtuse angle, then triangle is visible*/
			if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
			{
				// Illumination
				vec3d light_direction = { 0.0f, 1.0f, -1.0f };	// More real world "sunlight source"
				light_direction = Vector_Normalise(light_direction);

				// How "aligned" is light direction and triangle surface normal?
				// 0.1f--> ambient light	
				float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

				// Choose console colours as required (much easier with RGB)
				CHAR_INFO c = GetColour(dp);
				triTransformed.col = c.Attributes;
				triTransformed.sym = c.Char.UnicodeChar;

				// Convert World Space --> View Space
				triViewed.p[0] = Matrix_MultiplyVector(triTransformed.p[0], matView);
				triViewed.p[1] = Matrix_MultiplyVector(triTransformed.p[1], matView);
				triViewed.p[2] = Matrix_MultiplyVector(triTransformed.p[2], matView);
				triViewed.col = triTransformed.col;	// This intermediate appearance info copying isn't
				triViewed.sym = triTransformed.sym;	// showed in javidx9's video.But engineeringly need add.

				/*Clip Viewed triangles against near plane
				This could form two additional triangles.*/
				triangle clipped[2];
				int nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f,0.0f,0.1f }, { 0.0f,0.0f,1.0f }, triViewed, clipped[0], clipped[1]);

				for (int n = 0; n < nClippedTriangles; n++)
				{
					// Project triangles from 3D --> 2D
					triProjected.p[0] = Matrix_MultiplyVector(clipped[n].p[0], matProj);
					triProjected.p[1] = Matrix_MultiplyVector(clipped[n].p[1], matProj);
					triProjected.p[2] = Matrix_MultiplyVector(clipped[n].p[2], matProj);
					triProjected.col = clipped[n].col;
					triProjected.sym = clipped[n].sym;

					// Scale into view, we moved the normalising into cartesian space
					// out of the matrix.vector functions from previous videos, so
					// do this manually
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					/*Due to constraint of olcConsoleGameEngine.h, when Rendering Ouput 'Easy Axises of Coordinates',
					(0,0) is on left-up top of screen,Instead of middle of screen.It means the excepted either (-1,0) or (0,-1),
					will draw inversely,As opsed to either horizontal left (-1,0) or vertical up (-1,0);[(1,0),(0,1)] is also inverse;*/
					// X/Y are inverted, So put them back
					triProjected.p[0].x *= -1.0f;
					triProjected.p[1].x *= -1.0f;// Moreover, Engine 3d part3 cpp file refers standard Right-hand libaries 
					triProjected.p[2].x *= -1.0f;// of vectors and matrices.As it's completely inverse(x/y) result to Left-Hand axis;
					triProjected.p[0].y *= -1.0f;
					triProjected.p[1].y *= -1.0f;// To keep consistency, it's good to maintain standard the libaries,
					triProjected.p[2].y *= -1.0f;// Just remember to debug and adjust correspondingly to rendering api constraint.

					// Offset verts into visible normalising space
					vec3d vOffsetView = { 1.0f,1.0f,0.0f };
					triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
					triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
					triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);
					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					// Store triangle for sorting
					vecTrianglesToRaster.push_back(triProjected);
				}

			}

		}

		// Sort triangles from back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
			});

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		/* Loop through all transformed, viewed, 
		Z-clipped, projected, and sorted triangles.*/
		for (auto& triToRaster : vecTrianglesToRaster)
		{
			/* Clip triangles against all four screen edges.This could yield
			a bunch of triangles, so create a queue that we traverse to ensure
			we only test new triangles generated against planes.*/
			triangle clipped[2];
			list<triangle> listTriangles;

			// Add initial triangle
			listTriangles.emplace_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles--)
				{	
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();

					/*Clip it against a plane.We only need to test each
					subsequent plane, against subsequent new triangles
					as all triangles after a plane clip are guaranteed
					to lie on the inside of the plane.J' like how this
					commit is almost completely and utterly justtified*/
					switch (p)
					{
					case 0:nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f }, test, clipped[0], clipped[1]); break;
					case 1:nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f,(float)ScreenHeight() - 1,0.0f }, { 0.0f,-1.0f,0.0f }, test, clipped[0], clipped[1]); break;
					case 2:nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f }, test, clipped[0], clipped[1]); break;
					case 3:nTrisToAdd = Triangle_ClipAgainstPlane({ (float)ScreenWidth() - 1,0.0f,0.0f }, { -1.0f,0.0f,0.0f }, test, clipped[0], clipped[1]); break;
					}

					/* Clipping may yield a variable number of triangles, so
					add these new ones to the back of the queue for subsequent
					clipping against next planes.*/
					for (int w = 0; w < nTrisToAdd; w++)
					{
						listTriangles.emplace_back(clipped[w]);
					}
				}
				nNewTriangles = listTriangles.size();
			}

			/* Draw transformed, viewed, Z-clipped, 
			projected, sorted, and ScreenSpace-clipped triangles*/
			for (auto& tri : listTriangles)
			{
				FillTriangle(tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y
					, tri.p[2].x, tri.p[2].y, tri.sym, tri.col);

				/*DrawTriangle(tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y
					, tri.p[2].x, tri.p[2].y, PIXEL_SOLID, FG_BLACK);*/
			}
		}

		
		return true;
	}

};




int main()
{
	olcEngine3D demo;
	if (demo.ConstructConsole(256, 144, 4, 4))
		demo.Start();
	return 0;
}

