/*10.5-10.10 done,prepare to review this project in 2 days*/
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "olcConsoleGameEngine.h"

class FormulaOLC :public olcConsoleGameEngine {
public:
	FormulaOLC() {
		m_sAppName = L"Formula_OLC";
	}
private:
	float fCarPos = 0.0f;
	float fDistance = 0.0f;
	float fSpeed = 0.0f;

	float fCurvature = 0.0f;
	float fTrackCurvature = 0.0f;
	float fPlayerCurvature = 0.0f;
	float fTrackDistance = 0.0f;
	float fCurrentLapTime = 0.0f;

	vector<pair<float, float>> vecTrack;// curvature, distance
	list<float> listLapTimes;

protected:
	// Called by olcConsoleGameEngine
	virtual bool OnUserCreate() {
		vecTrack.emplace_back(0.0f, 10.0f);// Short section for start/finish line
		vecTrack.emplace_back(0.0f, 200.0f);
		vecTrack.emplace_back(1.0f, 200.0f);
		vecTrack.emplace_back(0.0f, 400.0f);
		vecTrack.emplace_back(-1.0f, 100.0f);
		vecTrack.emplace_back(0.0f, 200.0f);
		vecTrack.emplace_back(-1.0f, 200.0f);
		vecTrack.emplace_back(1.0f, 200.0f);
		vecTrack.emplace_back(0.0f, 200.0f);
		vecTrack.emplace_back(0.2f, 500.0f);
		vecTrack.emplace_back(0.0f, 200.0f);

		for (auto& x : vecTrack) {
			fTrackDistance += x.second;
		}

		listLapTimes = { 0,0,0,0,0 };

		return true;
	}
	// Called by olcConsoleGameEngine
	virtual bool OnUserUpdate(float fElapsedTime) {
		int nCarDirection = 0;
		
		if (m_keys[VK_UP].bHeld) {
			fSpeed += 2.0f * fElapsedTime;
		}
		else fSpeed -= 1.0f * fElapsedTime;

		if (m_keys[VK_LEFT].bHeld) {
			nCarDirection -= 1;
			fPlayerCurvature -= 0.7f * fElapsedTime;
		}

		if (m_keys[VK_RIGHT].bHeld) {
			nCarDirection += 1;
			fPlayerCurvature += 0.7f * fElapsedTime;
		}

		if (fabs(fTrackCurvature - fPlayerCurvature) >= 0.8f) {
			fSpeed -= 5.0f * fElapsedTime;
		}

		// Clamp Speed
		if (fSpeed < 0.0f)fSpeed = 0.0f;
		if (fSpeed > 1.0f)fSpeed = 1.0f;

		// Move car along track according to car speed
		fDistance += (70.0f * fSpeed) * fElapsedTime;

		// Get Point on Track
		float fOffset = 0.0f;
		int nTrackSection = 0;

		fCurrentLapTime += fElapsedTime;
		if (fDistance >= fTrackDistance) {
			fDistance -= fTrackDistance;
			listLapTimes.emplace_front(fCurrentLapTime);
			listLapTimes.pop_back();
			fCurrentLapTime = 0.0f;
		}

		// Find position on track (could optimise)
		while (nTrackSection < vecTrack.size() && fOffset <= fDistance) {
			fOffset += vecTrack[nTrackSection].second;
			nTrackSection++;
		}

		float fTargetCurvature = vecTrack[nTrackSection - 1].first;	
		float fTrackCurveDiff = (fTargetCurvature - fCurvature) * fElapsedTime * fSpeed;

		fCurvature += fTrackCurveDiff;
		fTrackCurvature += (fCurvature) * fElapsedTime * fSpeed;

		// Draw Sky - Light blue and Dark blue
		for (int y = 0; y < ScreenHeight() / 2; y++) {
			for (int x = 0; x < ScreenWidth(); x++) {
				Draw(x, y, y < ScreenHeight() / 4 ? PIXEL_HALF : PIXEL_SOLID, FG_BLUE);
			}
		}

		// Draw Scenery - our hills are a rectified sine wave, where the phase is adjusted by
		// accumulate track curvature
		for (int x = 0; x < ScreenWidth(); x++) {
			int nHillHeight = fabs(sinf(x * 0.01f + fTrackCurvature) * 16.0f);
			for (int y = ScreenHeight() / 2 - nHillHeight; y < ScreenHeight() / 2; y++) {
				Draw(x, y, PIXEL_SOLID, FG_DARK_YELLOW);
			}
		}

		for (int y = 0; y < ScreenHeight() / 2; y++) {
			for (int x = 0; x < ScreenWidth(); x++) {
				float fPerspective = (float)y / (ScreenHeight() / 2.0f);

				float fMiddlePoint = 0.5f + fCurvature * powf(1.0f - fPerspective, 3);
				float fRoadWidth = 0.1f + fPerspective * 0.8f;
				float fClipWidth = fRoadWidth * 0.15f;

				fRoadWidth *= 0.5f;

				int nLeftGrass = (fMiddlePoint - fRoadWidth - fClipWidth) * ScreenWidth();
				int nLeftClip = (fMiddlePoint - fRoadWidth) * ScreenWidth();
				int nRightClip = (fMiddlePoint + fRoadWidth) * ScreenWidth();
				int nRightGrass = (fMiddlePoint + fRoadWidth + fClipWidth) * ScreenWidth();

				int nRow = ScreenHeight() / 2 + y;

				int nGrassColour = sinf(20.0f * powf(1.0f - fPerspective, 3) + 0.1f * fDistance) > 0.0f ? FG_GREEN : FG_DARK_GREEN;
				int nClipColour = sinf(80.0f * powf(1.0f - fPerspective, 2) + fDistance) > 0.0f ? FG_RED : FG_WHITE;

				int nRoadColour = (nTrackSection - 1) == 0 ? FG_WHITE : FG_GREY;

				if (x >= 0 && x < nLeftGrass)Draw(x, nRow, PIXEL_SOLID, nGrassColour);
				if (x >= nLeftGrass && x < nLeftClip)Draw(x, nRow, PIXEL_SOLID, nClipColour);
				if (x >= nLeftClip && x < nRightClip)Draw(x, nRow, PIXEL_SOLID, nRoadColour);
				if (x >= nRightClip && x < nRightGrass)Draw(x, nRow, PIXEL_SOLID, nClipColour);
				if (x >= nRightGrass && x < ScreenWidth())Draw(x, nRow, PIXEL_SOLID, nGrassColour);
			}
		}
		
		// Draw Car
		fCarPos = fPlayerCurvature - fTrackCurvature;
		int nCarPos = (ScreenWidth() / 2) + ((int)(ScreenWidth() * fCarPos) / 2.0f) - 7;

		switch (nCarDirection) {
		case 0:
			DrawStringAlpha(nCarPos, 80, L"   ||####||   ");
			DrawStringAlpha(nCarPos, 81, L"      ##      ");
			DrawStringAlpha(nCarPos, 82, L"     ####     ");
			DrawStringAlpha(nCarPos, 83, L"     ####     ");
			DrawStringAlpha(nCarPos, 84, L"|||  ####  |||");
			DrawStringAlpha(nCarPos, 85, L"|||########|||");
			DrawStringAlpha(nCarPos, 86, L"|||  ####  |||");
			break;
		case +1:
			DrawStringAlpha(nCarPos, 80, L"      //####//");
			DrawStringAlpha(nCarPos, 81, L"         ##   ");
			DrawStringAlpha(nCarPos, 82, L"       ####   ");
			DrawStringAlpha(nCarPos, 83, L"      ####    ");
			DrawStringAlpha(nCarPos, 84, L"///  ####//// ");
			DrawStringAlpha(nCarPos, 85, L"//#######///O ");
			DrawStringAlpha(nCarPos, 86, L"/// #### //// ");
			break;
		case -1:
			DrawStringAlpha(nCarPos, 80, L"\\\\####\\\\      ");
			DrawStringAlpha(nCarPos, 81, L"   ##         ");
			DrawStringAlpha(nCarPos, 82, L"   ####       ");
			DrawStringAlpha(nCarPos, 83, L"    ####      ");
			DrawStringAlpha(nCarPos, 84, L" \\\\\\\\####  \\\\\\");
			DrawStringAlpha(nCarPos, 85, L" O\\\\\\#######\\\\");
			DrawStringAlpha(nCarPos, 86, L" \\\\\\\\ #### \\\\\\");
		}

		// Draw State
		DrawString(0, 0, L"Distance: " + to_wstring(fDistance));
		DrawString(0, 1, L"Target Curvature " + to_wstring(fCurvature));
		DrawString(0, 2, L"Player Curvature " + to_wstring(fPlayerCurvature));
		DrawString(0, 3, L"Player Speed " + to_wstring(fSpeed));
		DrawString(0, 4, L"Track Curvature " + to_wstring(fTrackCurvature));

		auto disp_time = [](float t) {
			int nMinutes = t / 60.0f;
			int nSeconds = t - nMinutes * 60.0f;
			int nMilliSecond = (t - (float)(nSeconds + nMinutes * 60.0f)) * 1000.0f;
			return to_wstring(nMinutes) + L"." + to_wstring(nSeconds) + L":" + to_wstring(nMilliSecond);
			};

		DrawString(10, 8, disp_time(fCurrentLapTime));

		// Display Last 5 LapTime
		int j = 10;
		for (const auto& l : listLapTimes) {
			DrawString(10, j, disp_time(l));
			j++;
		}

		return true;
	}
};

int main(void) {
	// Use ConsoleGameEngine derived app
	FormulaOLC game;
	game.ConstructConsole(160, 100, 6, 6);
	game.Start();
	return 0;
}