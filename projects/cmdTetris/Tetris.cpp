/*Condensed English Version
This project was developed between September 2 and September 13, 2025, with additional review and cleanup on September 14–15. 
It represents my first substantial C++ learning project after installing Microsoft Visual Studio Community 2022 on September 2, 2025.

Before starting, I spent time on September 1 searching for comprehensive C++ learning resources and 
eventually chose the tutorial series by javidx9 (OneLoneCoder). The videos focus directly on code implementation without unnecessary detours, 
so I relied on both the tutorial and the corresponding GitHub source code to understand certain details—especially parts involving 
system‑level components such as windows.h, which the videos treat as secondary tools rather than core teaching material.

Following the project step by step introduced many new concepts, but I was able to connect them with the C++ fundamentals 
I had strengthened earlier through problem‑solving practice. 
Completing this project marked a clear stage of growth in my understanding of practical C++ programming and project structure.*/
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
using namespace std;
using namespace std::chrono_literals;
#include <Windows.h>

int nScreenWidth = 120;		//Console Screen Size X(columns) ori:80
int nScreenHeight = 30;		//Console Screen Size Y(rows) ori:30
wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int Rotate(int px, int py, int r) {
	int pi = 0;
	switch (r % 4) {
		case 0://0 degrees				// 0  1  2  3 
			pi = py * 4 + px; break;	// 4  5  6  7 
										// 8  9 10 11
										//12 13 14 15
		
		case 1://90 degrees					// 3  7 11 15
			pi = (3 - px) * 4 + py; break;	// 2  6 10 14
											// 1  5  9 13
											// 0  4  8 12

		case 2://180 degrees					 //15 14 13 12
			pi = (3 - py) * 4 + (3 - px); break; //11 10  9  8
												 // 7  6  5  4
												 // 3  2  1  0
		
		case 3://270 degrees				// 3  7 11 15
			pi = px * 4 + (3 - py); break;	// 2  6 10 14
	}										// 1  5  9 13
											// 0  4  8 12

	return pi;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {

	//All Field cells >0 are occupied
	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {

			//Get the index into piece
			int pi = Rotate(px, py, nRotation);

			//Get the index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			/*Check that test is in bounds.Note out of bounds does
			not necessarily mean a fail,as the long vertical piece
			can have cells that lie outside the boundary,so we'll
			just ignore them----copied from source github file*/
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {

					//In Bounds so do collision check
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0) {
						return false; //fail on first hit
					}
				}
			}
		}
	}

	return true;
}

int main(void) {

	srand(static_cast<unsigned>(time(nullptr)));

	ios::sync_with_stdio(false);
	cin.tie(nullptr);

	// Create screen buffer
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenHeight * nScreenWidth; i++)screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Create assets
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX..X......");
	tetromino[2].append(L".X...XX...X.....");
	tetromino[3].append(L".....XX..XX.....");
	tetromino[4].append(L"..X..XX...X.....");
	tetromino[5].append(L".....XX...X...X.");
	tetromino[6].append(L".....XX..X...X..");

	pField = new unsigned char[nFieldHeight * nFieldWidth];//Create play field buffer
	for (int x = 0; x < nFieldWidth; x++) {//Board Boundary
		for (int y = 0; y < nFieldHeight; y++) {
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	//Game Logic stuff
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	int nScore = 0;
	int nPieceCount = 0;
	vector<int> vLines;
	
	bool bGameOver = false;
	bool bKey[4];
	bool bRotateHold = false;
	bool bForceDwon = false;

	while (!bGameOver) { //Main Loop
		// TIMING ===========================
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		bForceDwon = (nSpeed == nSpeedCount);
		
		// INPUT ============================
		for (int k = 0; k < 4; k++) {							// R   L   D  Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		}

		// GAME LOGIC =======================

		// Handle player movement ===========
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else bRotateHold = false;

		if (bForceDwon) {
			// Update diffculty every 50 pieces
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0) {
				if (nSpeed >= 10)nSpeed--;
			}
			// Test if piece can be moved on
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;// It can, so do it!
			}
			else {
				// Lock the piece in the field
				for (int px = 0; px < 4; px++) {
					for (int py = 0; py < 4; py++) {
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}

				// Check have we got any lines
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove Line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				// Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// If piece does not fit
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		// RENDER OUTPUT ====================

		// Draw Field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}
		
		// Draw Current Piece
		for (int px = 0; px < 4; px++) {
			for (int py = 0; py < 4; py++) {
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
				}
			}
		}

		// Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		// Animate Line Completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // Delay a bit

			for (auto& v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	CloseHandle(hConsole);
	cout << "Game Over!!! Score:" << nScore << endl;
	system("pause"); return 0;
}