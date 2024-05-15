#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <random>
#include <time.h>
using namespace std;

wstring tertromino[7];
int nFieldWitdth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int nScreenWidth = 80;
int nScreenHeigth = 30;

bool bGameOver = false;

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;			// 0 degrees
	case 1: return 12 + py - (px * 4);  // 90 degrees
	case 2: return 15 - (py * 4) - px;  // 180 degrees
	case 3: return 3 - py + (px * 4);   //270 degrees
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWitdth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < nFieldWitdth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (tertromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false;//  fail on first hit
				}
			}
		}
	}

	return true;
}


int main()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	//Create assets

	tertromino[0].append(L"..X.");
	tertromino[0].append(L"..X.");
	tertromino[0].append(L"..X.");
	tertromino[0].append(L"..X.");
	
	tertromino[1].append(L"..X.");
	tertromino[1].append(L".XX.");
	tertromino[1].append(L".X..");
	tertromino[1].append(L"....");
	
	tertromino[2].append(L".X..");
	tertromino[2].append(L".XX.");
	tertromino[2].append(L"..X.");
	tertromino[2].append(L"....");
	
	tertromino[3].append(L"....");
	tertromino[3].append(L".XX.");
	tertromino[3].append(L".XX.");
	tertromino[3].append(L"....");
	
	tertromino[4].append(L"..X.");
	tertromino[4].append(L".XX.");
	tertromino[4].append(L"..X.");
	tertromino[4].append(L"....");
	
	tertromino[5].append(L"....");
	tertromino[5].append(L".XX.");
	tertromino[5].append(L"..X.");
	tertromino[5].append(L"..X.");

	tertromino[6].append(L"....");
	tertromino[6].append(L".XX.");
	tertromino[6].append(L".X..");
	tertromino[6].append(L".X..");

	pField = new unsigned char[nFieldHeight * nFieldWitdth];
	for (int x = 0; x < nFieldWitdth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			pField[y * nFieldWitdth + x] = (x == 0 || x == nFieldWitdth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeigth];
	for (int i = 0; i < nScreenWidth * nScreenHeigth; i++)
		screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//  Game Logic Stuff
	int nCurrentPiece = 1;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWitdth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHold = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

	while (!bGameOver)
	{
		//	GAME TIMING =======================================
		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);
		
		//	INPUT =============================================
		for(int k = 0;k<4;k++)									 // R  L   D  Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
		

		//	GAME LOGIC ========================================
		//	Player Control
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;

		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))? 1 : 0;

		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		if (bKey[3])
		{
			nCurrentRotation += (!bRotateHold&&DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else
		{
			bRotateHold = false;
		}

		//	Auto Logic
		if (bForceDown)
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else
			{
				// Lock the current piece in the field
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tertromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py) * nFieldWitdth + nCurrentX + px] = nCurrentPiece + 1;

				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				// Check have we got any lines
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWitdth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWitdth + px]) != 0;

						if (bLine)
						{
							for (int px = 1; px < nFieldWitdth - 1; px++)
								pField[(nCurrentY + py) * nFieldWitdth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;
				
				// Choose next piece
				nCurrentX = nFieldWitdth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = gen() % 7;

				// if peice does not fit
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			nSpeedCounter = 0;
		}
		//  RENDER OUTPUT =====================================

		//  Draw Field
		for (int x = 0; x < nFieldWitdth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWitdth + x]];
			}
		}

		//  Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tertromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
		
		// Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldHeight + 6], 16, L"SCORE:%8d", nScore);

		if (!vLines.empty())
		{
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeigth, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto& v : vLines)
			{
				for (int px = 1; px < nFieldWitdth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWitdth + px] = pField[(py - 1) * nFieldWitdth + px];
					pField[px] = 0;
				}
			}
			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeigth, { 0,0 }, &dwBytesWritten);
	}

	CloseHandle(hConsole);
	cout << "Game Over!! Score:" << nScore << endl;
	system("pause");

	return 0;
}

