/*Eventually, i have to do a handcraft for these cpp project or ill never master
complete during 2025.10.12-2025.10.16;2 extra gameplay*/
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
using namespace std;
using namespace chrono_literals;
#include <Windows.h>

int nFieldHeight = 18;
int nFieldWidth = 12;
unsigned char* pField = nullptr;
wstring tetromino[7];
int nScreenHeight = 30;
int nScreenWidth = 120;
int nBest = 0;

int Rotate(int R, int x, int y) {
	int pi = 0;
	switch (R) {
	case 0:		// 0 degrees
		pi=y * 4 + x; break;
	case 1:		// 90 degrees
		pi=(3 - x) * 4 + y; break;
	case 2:		// 180 degrees
		pi=(3 - y) * 4 + (3 - x); break;
	case 3:		// 270 degrees
		pi=x * 4 + (3 - y); break;
	default:break;
	}
	return pi;
}

bool DoesPieceFit(int nTetris, int nRotation, int px, int py) {
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int pi = Rotate(nRotation, x, y);	//Index in tetromino[nTetris]
			int fi = (py + y) * nFieldWidth + (px + x);	//Index in Field
			if (px + x >= 0 && px + x < nFieldWidth) {	// Column boundary check
				if (py + y >= 0 && py + y < nFieldHeight) {	// Row boundary check
					if (tetromino[nTetris][pi] != L'.' && pField[fi] != 9) {
						return false;	// Collision detective
					}
				}
			}
		}
	}

	return true;
}

int main(void) {

	// Initialize seed
	srand(static_cast<unsigned int>(time(NULL)));
	// Improve programming performance
	ios::sync_with_stdio(false); cin.tie(nullptr);
	
	//initialize console screen buffer
	wchar_t* screen = new wchar_t [nScreenHeight * nScreenWidth];
	for (int x = 0; x < nScreenHeight * nScreenWidth; x++)screen[x] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_WRITE | GENERIC_READ, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	
	// Create tetromino assets
	tetromino[0].append(L"..#.");
	tetromino[0].append(L"..#.");
	tetromino[0].append(L"..#.");
	tetromino[0].append(L"..#.");

	tetromino[1].append(L".#..");
	tetromino[1].append(L".##.");
	tetromino[1].append(L"..#.");
	tetromino[1].append(L"....");

	tetromino[2].append(L".##.");
	tetromino[2].append(L".##.");
	tetromino[2].append(L"....");
	tetromino[2].append(L"....");

	tetromino[3].append(L".#..");
	tetromino[3].append(L".##.");
	tetromino[3].append(L".#..");
	tetromino[3].append(L"....");

	tetromino[4].append(L".#..");
	tetromino[4].append(L".#..");
	tetromino[4].append(L".##.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"..#.");
	tetromino[5].append(L".##.");
	tetromino[5].append(L".#..");
	tetromino[5].append(L"....");
	
	tetromino[6].append(L"..#.");
	tetromino[6].append(L"..#.");
	tetromino[6].append(L".##.");
	tetromino[6].append(L"....");

	// Create player buffer
	pField = new unsigned char[nFieldHeight * nFieldWidth];
	for (int x = 0; x < nFieldWidth; x++) {
		for (int y = 0; y < nFieldHeight; y++) {	// Set boundary 
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 0 : 9;
		}
	}

	// Game Logic Stuff
	int nTetris = 0;
	int nNewTetris = rand() % 7;
	int nNTx = 10 + nFieldWidth;
	int nNTy = 7;
	int nRotation = 0;
	int nCurrentX = (nFieldWidth / 2) - 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	int nScore = 0;

	bool bForceDown = false;
	bool bGameOver = false;
	bool bKeys[4];
	bool bHeldZ = false;
	vector<int> vLines;
	

	while (!bGameOver) {
		//-------------TIMING--------------
		this_thread::sleep_for(50ms);// smooth the effect when game logic update
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);
		
		//--------------INPUT--------------

		// Get key State
		for (int i = 0; i < 4; i++) {			  //L	R   D  Z
			bKeys[i] = (0x8000 & GetAsyncKeyState("\x25\x27\x28Z"[i])) != 0;
		}
		// Handle tetris movement
		nCurrentX -= (bKeys[0] && DoesPieceFit(nTetris, nRotation, nCurrentX-1, nCurrentY)) ? 1 : 0;
		nCurrentX += (bKeys[1] && DoesPieceFit(nTetris, nRotation, nCurrentX+1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKeys[2] && DoesPieceFit(nTetris, nRotation, nCurrentX, nCurrentY+1)) ? 1 : 0;

		if (!bHeldZ && bKeys[3]) {
			nRotation = (DoesPieceFit(nTetris, nRotation + 1, nCurrentX, nCurrentY)) ? (nRotation + 1) % 4 : nRotation;
			bHeldZ = true;
		}
		else bHeldZ = bKeys[3];

		// Nature logic
		if (bForceDown) {

			// Reset nature fall counter
			nSpeedCount = 0;
			// Update diffculty every 1000 scores
			nSpeed -= ((nScore % 1000 == 0 && nSpeed >= 10)) ? 1 : 0;

			if (DoesPieceFit(nTetris, nRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++;
			}
			else {
				// Lock piece into pfield
				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						wchar_t c = tetromino[nTetris][Rotate(nRotation, x, y)];
						if (c != L'.')pField[(nCurrentY + y) * nFieldWidth + (nCurrentX + x)] = nTetris + 1;
					}
				}

				//Check if got any lines
				for (int y = 0; y < 4; y++) {
					if (nCurrentY + y < nFieldHeight - 1) {
						bool bLine = true;
						for (int x = 1; x < nFieldWidth - 1; x++) {
							bLine &= (pField[(nCurrentY + y) * nFieldWidth + x] != 9);
						}
						if (bLine) {

							// Set the line to '='
							for (int x = 1; x < nFieldWidth - 1; x++) {
								pField[(nCurrentY + y) * nFieldWidth + x] = 8;
							}

							// Store y coordinate of the line
							vLines.emplace_back(nCurrentY + y);
						}
					}
				}			

				// Accumulate 25 scores per lock, more awards when game got lines
				nScore += 25;
				if (!vLines.empty())nScore += (1 << vLines.size()) * 100;

				// Reset tetris
				nCurrentX = (nFieldWidth / 2) - 2;
				nCurrentY = 0;
				nRotation = 0;
				nTetris = nNewTetris;
				nNewTetris = rand() % 7;

				// GameOver when pField can't hold any tetris to move
				bGameOver = !(DoesPieceFit(nTetris, nRotation, nCurrentX, nCurrentY));
			}
		}
		
		//----------RENDER OUTPUT----------
		
		// Draw play field into console screen
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				// Play Field
				screen[(y + 3) * nScreenWidth + (x + 3)] = L"#ABCDEFG= "[pField[y * nFieldWidth + x]];
				// UI Field of Showing Next Tetris
				if (x < 4 && y < 4) {
					screen[(nNTy + y) * nScreenWidth + (nNTx + x)] = L' ';
				}
			}
		}

		// Draw tetris into console screen
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				// Current Piece
				if (tetromino[nTetris][Rotate(nRotation, x, y)] != L'.') {
					screen[(nCurrentY + y + 3) * nScreenWidth + (nCurrentX + x + 3)] = nTetris + 65;
				}
				// Next Piece
				if (tetromino[nNewTetris][Rotate(0, x, y)] != L'.') {
					screen[(nNTy + y) * nScreenWidth + (nNTx + x)] = nNewTetris + 65;
				}
			}
		}

		// Display player score
		swprintf_s(&screen[nScreenWidth * 3 + nFieldWidth + 6], 13, L"Score:%6d", nScore);

		// Display text of noticing next Tetris
		swprintf_s(&screen[nScreenWidth * 5 + nFieldWidth + 6], 13, L"Next Tetris;");

		// Animated disapearance of lines
		if (!vLines.empty()) {
			// Display Frame(checkily to draw lines)
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);	// Delay a bit

			for (const int& ly : vLines) {
				for (int x = 1; x < nFieldWidth - 1; x++) {
					for (int y = ly; y > 0; y--) {
						pField[y * nFieldWidth + x] = pField[(y - 1) * nFieldWidth + x];
					}
					pField[0 * nFieldWidth + x] = 9;
				}
			}
			// Clear vLines
			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	// Exit | Play again
	CloseHandle(hConsole); nBest = max(nScore, nBest);
	cout << "Game Over! Current Score: " << nScore << endl;
	cout << "Highest Score since from start playing: " << nBest << endl;
	cout << "Press R to restart or E to exit." << endl;
	system("pause"); int nRcount = 10; bool bRE[2];
	
	while (nRcount) {
		cout << "\rExit in " << nRcount << " s" << flush;nRcount--;
		this_thread::sleep_for(500ms);
		for (int i = 0; i < 2; i++) {
			bRE[i] = (0x8000 & GetAsyncKeyState("RE"[i])) != 0;
			this_thread::sleep_for(240ms);
		}
		
		if (bRE[0]) main();	// Restart
		else if (bRE[1]) break;	// Exit
	}
	delete[] screen,pField;return 0;
}