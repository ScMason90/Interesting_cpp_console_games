/*Finished during 11.23-11.25, Review on 11.27*/
#include <algorithm>
#include <iostream>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"

class olc_Asteroids :public olcConsoleGameEngine
{
public:
	olc_Asteroids()
	{
		m_sAppName = L"Reproduct_Asteroids";
	}

private:
	struct sSpaceObject
	{
		float x;
		float y;
		float dx;
		float dy;
		int nSize;
		float angle;
	};

	vector<sSpaceObject> vecAsteroids;
	vector<sSpaceObject> vecBullets;
	sSpaceObject player;
	int nScore = 0;
	bool bDead = false;

	vector<pair<float, float>> vecModelShip;
	vector<pair<float, float>> vecModelAsteroid;

protected:
	// Called by olcConsoleGameEngine
	virtual bool OnUserCreate()
	{
		vecModelShip =
		{
			{0.0f,  -5.0f},
			{-2.5f, +2.5f},
			{+2.5f, +2.5f}
		};	// A small Isoceles Triangle

		int verts = 20;
		for (int i = 0; i < verts; i++)
		{
			float radius = ((float)rand() / (float)RAND_MAX) * 0.4f + 0.8f;
			float a = ((float)i / (float)verts) * 3.14159f * 2.0f;
			vecModelAsteroid.push_back(make_pair(radius * sinf(a), radius * cosf(a)));
		}

		ResetGame();

		return true;
	}

	bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrtf(powf(cx - x, 2) + powf(cy - y, 2)) < radius;
	}

	void ResetGame()
	{
		vecAsteroids.clear();
		vecBullets.clear();

		vecAsteroids.push_back({ 20.0f,20.0f,8.0f,-6.0f,(int)16,0.0f });
		vecAsteroids.push_back({ 100.0f,20.0f,-5.0f,3.0f,(int)16,0.0f });

		// Initialise Player Position 
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0;

		bDead = false;
		nScore = 0;
	}

	// Called by olcConsoleGameEngine
	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (bDead)
		{
			ResetGame();
		}

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		// Steer
		if (m_keys[VK_LEFT].bHeld)player.angle -= 5.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)player.angle += 5.0f * fElapsedTime;

		// Thrust
		if (m_keys[VK_UP].bHeld)
		{
			// ACCELEREATION changes VELOCITY(with respect to time)
			player.dx += sinf(player.angle) * 20.0f * fElapsedTime;
			player.dy -= cosf(player.angle) * 20.0f * fElapsedTime;
		}

		// VELOCITY changes POSITION(with respect to time)
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		// Keep ship in game space
		WrapCoordinates(player.x, player.y, player.x, player.y);

		// Check ship collision with asteroids
		for (auto& a : vecAsteroids)
		{
			if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
			{
				bDead = true;
			}
		}

		// Fire Bullet in direction of player
		if (m_keys[VK_SPACE].bReleased) 
		{
			vecBullets.push_back({ player.x,player.y,50.0f * sinf(player.angle),50.0f * -cosf(player.angle),0,0 });
		}

		// Update and draw Asteroids
		for (auto& a : vecAsteroids)
		{
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			if (a.x > a.y)a.angle += 0.5f * fElapsedTime;
			else a.angle -= 1.0f * fElapsedTime;
			WrapCoordinates(a.x, a.y, a.x, a.y);

			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, a.nSize, FG_YELLOW);

		}

		vector<sSpaceObject> vecNewAsteroids;

		// Update bullets
		for (auto& b : vecBullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			//WrapCoordinates(b.x, b.y, b.x, b.y);

			// Check collision with asteroids
			for (auto& a : vecAsteroids)
			{
				if (IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					// Asteroid hit
					b.x -= 100.0f;	b.y -= 100.0f;

					if (a.nSize > 4)
					{	
						// Create two child asteroids (random angle of acceleration)
						float angle1 = ((float)rand() / (float)RAND_MAX) * 3.14159f * 2.0f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 3.14159f * 2.0f;
						vecNewAsteroids.push_back({ a.x,a.y,10.0f * sinf(angle1),10.0f * cosf(angle1),(int)a.nSize >> 1,0.0f });
						vecNewAsteroids.push_back({ a.x,a.y,10.0f * sinf(angle2),10.0f * cosf(angle2),(int)a.nSize >> 1,0.0f });
					}

					// Destroy hit asteroid
					a.x -= 100.0f;	a.y -= 100.0f;
					nScore += 100;
				}
			}
		}

		// Append new asteroids to existing vector
		for (auto& a : vecNewAsteroids)
		{
			vecAsteroids.emplace_back(a);
		}

		// Remove off dead bullets
		if (vecBullets.size() > 0)
		{
			/*auto i = remove_if(vecBullets.begin(), vecBullets.end(),
				[&](sSpaceObject o)
				{
					return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() || o.y >= ScreenHeight());
				});
			if (i != vecBullets.end()) vecBullets.erase(i, vecBullets.end());*/
			vecBullets.erase(remove_if(vecBullets.begin(), vecBullets.end(),
				[&](sSpaceObject o) {
					return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() || o.y >= ScreenHeight());
				}), vecBullets.end());
		}
		
		// Remove hit and dead asteroids
		if (vecAsteroids.size() > 0)
		{	// MSN Copilot optimised template instead of Javidx9 source snippet
			vecAsteroids.erase(remove_if(vecAsteroids.begin(), vecAsteroids.end(),
				[&](sSpaceObject o)
				{
					return (o.x < 0 || o.y < 0);
				}), vecAsteroids.end());
		}

		if (vecAsteroids.empty())
		{
			nScore += 1000;
			vecAsteroids.clear();
			vecBullets.clear();

			/*Add two new asteroids, but in a place where the player is not, we'll simply
			add them 90 degress left and right to the player, their coordinates will
			be wrapped by the next asteroid update*/
			float left_90 = player.angle - 3.14159f / 2.0f, 
				right_90 = player.angle + 3.14159f / 2.0f;
			vecAsteroids.push_back({ 30.0f * sinf(left_90) + player.x,
									 30.0f * cosf(left_90) + player.y,
									 10.0f * sinf(player.angle),10.0f * cosf(player.angle),(int)16,0.0f });
			vecAsteroids.push_back({ 30.0f * sinf(right_90) + player.x,
									 30.0f * cosf(right_90) + player.y,
									 10.0f * sinf(player.angle),10.0f * cosf(player.angle),(int)16,0.0f });
		}

		// Draw Bullets
		for (auto& b : vecBullets)Draw(b.x, b.y);

		// Draw Ship?
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

		// Draw Score
		DrawString(2, 2, L"SCORE: " + to_wstring(nScore));
		
		return true;
	}

	void WrapCoordinates(float ix, float iy, float& ox, float& oy)
	{
		/*ox = ((int)ix % ScreenWidth() + ScreenWidth()) % ScreenWidth();
		oy = ((int)iy % ScreenHeight() + ScreenHeight()) % ScreenHeight();*/
		ox = ix;	float nWidth = (float)ScreenWidth();
		oy = iy;	float nHeight = (float)ScreenHeight();
		if (ox < 0.0f)ox += nWidth;	if (ox >= nWidth)ox -= nWidth;
		if (oy < 0.0f)oy += nHeight;if (oy >= nHeight)oy -= nHeight;
	}

	virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}

	void DrawWireFrameModel(const vector<pair<float, float>>& vecModelCoordinates, float x, float y, float r, float s = 1.0f, float col = FG_WHITE)
	{
		// pair.first = x coordinate
		// pair.second = y coordinate

		// Create transformed model vector of coordinate pairs
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotate
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Scale
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first *= s;
			vecTransformedCoordinates[i].second *= s;
		}

		// Translate
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first += x;
			vecTransformedCoordinates[i].second += y;
		}

		// Draw Closed Polygon
		for (int i = 0; i < verts; i++)
		{
			int j = (i + 1) % verts; i %= verts;
			DrawLine(vecTransformedCoordinates[i].first,
				vecTransformedCoordinates[i].second,
				vecTransformedCoordinates[j].first,
				vecTransformedCoordinates[j].second, PIXEL_SOLID, col);
		}
		/*for (int i = 0; i <= verts; i++)
		{
			int j = i + 1;
			DrawLine(vecTransformedCoordinates[i % verts].first,
				vecTransformedCoordinates[i % verts].second,
				vecTransformedCoordinates[j % verts].first,
				vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}*/
	}
};

int main(void)
{
	// Use olcConsoleGameEngine derived up
	olc_Asteroids game;
	game.ConstructConsole(160, 100, 6, 6);// RetroArcade;256, 144, 4, 4
	game.Start();
	return 0;
}