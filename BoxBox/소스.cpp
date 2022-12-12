#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "msimg32.lib")
#pragma comment (lib, "winmm.lib")

#include "resource.h"
#include <windows.h>
#include <math.h>
#include <time.h>
#include <mmsystem.h>//PlaySound, mciSendCommand
#include <Digitalv.h>//mciSendCommand
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>

#pragma region 변수

MCI_PLAY_PARMS playBgm;
MCI_OPEN_PARMS openBgm;
int dwID;

HWND hWndMain, hEdit;

WNDPROC OldEditProc1, OldEditProc2;

HBITMAP hBit, BoxBit[4], BoxMoveBit[20], TileBit[3], BackgroundBit, TitleBit, ArrowBit, RankingBit;

int px = 0, py = 150;		// 박스의 위치
int pfrm = 0;				// 박스 프레임
int pArrx = 0, pArry = 0;				// 박스 배열위치

int motion = 0;		// 이동 애니메이션

enum { UP, RIGHT, DOWN, LEFT };			// 위아래옆

int BoxState = 0;		// 박스의 상태 5까지

bool move = false;		// 움직이고 있는 지 체크

int titleSelect = 1;			// 타이틀에서 선택한 버튼

int stage = 0;		// 현재 스테이지

int minute = 0;		// 분 타이머
int second = 0;		// 초 타이머
bool start = false;		// 타이머 움직일 지

bool dbcheck = false;		// 랭킹 보이게 할 지

bool editcheck = true;		// 서브클래스 보이게 할 지

#pragma endregion

class Rank
{
public:
	void SetID(TCHAR* m)
	{
		strcpy((TCHAR*)ID, m);
	}

	int num = 0;
	SQLCHAR ID[30] = "";
	int minute = 0;
	int second = 0;
	bool check = false;
};

Rank rank[10];

#pragma region DB관련

SQLHENV hEnv;
SQLHDBC hDbc;
SQLHSTMT hStmt;

#pragma endregion

#pragma region Maps

// 0 = 골인 타일, 1 = 타일1, 2 = 타일2, 음수 제외 나머지 숫자 = 빈칸
int map[7][7] =
{
	{2,3,3,3,3,0,2},
	{1,3,3,3,3,2,1},
	{2,1,2,1,2,1,2},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3}
};

int map1[7][7] =
{
	{2,3,3,3,3,0,2},
	{1,3,3,3,3,2,1},
	{2,1,2,1,2,1,2},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3}
};

int map2[7][7] =
{
	{2,0,3,3,3,3,3},
	{1,3,3,3,3,3,3},
	{2,1,2,1,3,3,3},
	{3,2,1,2,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3}
};

int map3[7][7] =
{
	{2,1,3,1,0,3,3},
	{1,2,1,2,1,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3}
};

int map4[7][7] =
{
	{2,1,2,3,3,3,3},
	{1,2,1,2,1,2,0},
	{2,1,2,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3}
};

int map5[7][7] =
{
	{0,1,2,3,3,3,3},
	{3,2,1,2,3,3,3},
	{3,3,2,1,2,1,3},
	{3,3,3,3,1,2,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3},
	{3,3,3,3,3,3,3}
};


int map6[7][7] =
{
	{2,1,2,3,2,1,2},
	{1,2,1,3,1,3,1},
	{2,1,2,3,2,3,2},
	{3,2,3,3,1,3,1},
	{3,1,3,0,2,3,2},
	{3,2,3,3,3,3,1},
	{3,1,2,1,2,1,2}
};

#pragma endregion

LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("성기정_18032042");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL; //MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPED | WS_SYSMENU,
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		550, 100, 700, 700,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	hWndMain = hWnd;

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}
// WINDOWS API


// 함수 선언
#pragma region 함수 선언

void BGMStart();
void UpdateUser();
BOOL DBConnect();
void DBDisConnect();
BOOL DBExecuteSQL(HDC hMemDC);
void SelectButton(int m);
void StageTile(HDC hMemDC);
void ClearCheck();
bool TileCheck(int m);
void TileBlt(HDC hMemDC, int x, int y, int m);
void BoxFrame();
void BoxMove(int m);
void MoveTransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask, int frm, int imgSizeX, int imgSizeY);
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask);
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
void OnTimer();
void Create();

#pragma endregion

// BGM 실행
void BGMStart()
{
	mciSendCommand(dwID, MCI_CLOSE, 0, NULL);//정지 및 메모리에서 삭제
	dwID = 0;

	openBgm.lpstrDeviceType = "mpegvideo";// wave , "mpegvideo"; //mp3 형식
	openBgm.lpstrElementName = "BGM.mp3";
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openBgm);
	dwID = openBgm.wDeviceID;
	mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&playBgm);
}

// 랭킹 등록
void UpdateUser()
{
	int Num, Minute, Second;
	SQLCHAR ID[30];

	SQLINTEGER lNum, lID, lMinute, lSecond;

	TCHAR szSQL[128];

	// 결과를 돌려받기 위해 바인딩한다.
	SQLBindCol(hStmt, 1, SQL_C_ULONG, &Num, 0, &lNum);
	SQLBindCol(hStmt, 2, SQL_C_CHAR, ID, 30, &lID);
	SQLBindCol(hStmt, 3, SQL_C_ULONG, &Minute, 0, &lMinute);
	SQLBindCol(hStmt, 4, SQL_C_ULONG, &Second, 0, &lSecond);

	// SQL문을 실행한다.
	if (SQLExecDirect(hStmt, (SQLCHAR*)"select * from Data", SQL_NTS) != SQL_SUCCESS)
	{
		return;
	}

	// 읽어온 데이터 출력
	int count = 0;
	while (SQLFetch(hStmt) != SQL_NO_DATA)
	{
		rank[count].num = Num;
		rank[count].SetID((TCHAR*)ID);
		rank[count].minute = Minute;
		rank[count].second = Second;
		rank[count].check = true;
		count++;
	};

	if (hStmt) SQLCloseCursor(hStmt);

	for (int i = 0; i < 10; i++)
	{
		if (rank[i].check)
		{
			wsprintf(szSQL, "Delete from Data where ID ='%s'", rank[i].ID);

			if (SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS) != SQL_SUCCESS)
			{
				MessageBox(hWndMain, "오류.", "오류", MB_OK);
			}
		}
	}

	if (hStmt) SQLCloseCursor(hStmt);

	for (int j = 0; j < 10; j++)
	{
		for (int i = 0; i < 10; i++)
		{
			if (rank[i + 1].check)
			{
				if (rank[i].minute > rank[i + 1].minute)		// 분이 작으면
				{
					Rank TempRank;

					TempRank = rank[i];
					rank[i] = rank[i + 1];
					rank[i + 1] = TempRank;			// 바로 바꾸기
				}
				else if (rank[i].minute >= rank[i + 1].minute)		// 같다면
				{
					if (rank[i].second > rank[i + 1].second)		// 초까지 확인
					{
						Rank TempRank;

						TempRank = rank[i];
						rank[i] = rank[i + 1];
						rank[i + 1] = TempRank;
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	// 정렬된 랭크 순위 매기기
	for (int i = 0; i < 10; i++)
	{
		if (rank[i].check)
		{
			rank[i].num = i + 1;
		}
	}

	// 정렬한 랭크 DB에 추가
	for (int i = 0; i < 10; i++)
	{
		if (rank[i].check)
		{
			wsprintf(szSQL, "Insert into Data (Num, ID, Minute, Second) VALUES (%d, '%s', %d, %d)", rank[i].num, rank[i].ID, rank[i].minute, rank[i].second);

			if (SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS) != SQL_SUCCESS)
			{
				MessageBox(hWndMain, "이름이 중복되어 랭킹등록에 실패하였습니다.", "오류", MB_OK);
				return;
			}
		}
	}

	MessageBox(hWndMain, "랭킹등록이 완료되었습니다!", "알림", MB_OK);

	if (hStmt) SQLCloseCursor(hStmt);
}

#pragma region DB관련

BOOL DBConnect()
{
	// 연결 설정을 위한 변수들
	SQLCHAR InCon[255];
	SQLCHAR OutCon[1024];
	SQLSMALLINT cbOutCon;
	TCHAR Dir[MAX_PATH];
	SQLRETURN Ret;

	// 환경 핸들을 할당하고 버전 속성을 설정한다.
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS)
		return FALSE;
	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3,
		SQL_IS_INTEGER) != SQL_SUCCESS)
		return FALSE;

	// 연결 핸들을 할당하고 연결한다.
	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS)
		return FALSE;

	// MDB 파일에 연결하기
	GetCurrentDirectory(MAX_PATH, Dir);
	wsprintf((TCHAR*)InCon, "DRIVER={Microsoft Access Driver (*.mdb)};"
		"DBQ=%s\\Ranking.mdb;", Dir);
	Ret = SQLDriverConnect(hDbc, hWndMain, InCon, sizeof(InCon), OutCon, sizeof(OutCon),
		&cbOutCon, SQL_DRIVER_NOPROMPT);

	if ((Ret != SQL_SUCCESS) && (Ret != SQL_SUCCESS_WITH_INFO))
		return FALSE;

	// 명령 핸들을 할당한다.
	if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS)
		return FALSE;

	return TRUE;
}

void DBDisConnect()
{
	// 뒷정리
	if (hStmt) SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	if (hDbc) SQLDisconnect(hDbc);
	if (hDbc) SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	if (hEnv) SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

BOOL DBExecuteSQL(HDC hMemDC)
{
	// 결과값을 돌려받기 위한 변수들
	SQLCHAR ID[128];
	int Num, Minute, Second;
	SQLINTEGER lNum, lID, lMinute, lSecond;
	// 화면 출력을 위한 변수들
	int y = 1;
	TCHAR str[255];

	RECT rt;

	HFONT font, oldfont;

	// 결과를 돌려받기 위해 바인딩한다.
	SQLBindCol(hStmt, 1, SQL_C_ULONG, &Num, 0, &lNum);
	SQLBindCol(hStmt, 2, SQL_C_CHAR, ID, 30, &lID);
	SQLBindCol(hStmt, 3, SQL_C_ULONG, &Minute, 0, &lMinute);
	SQLBindCol(hStmt, 4, SQL_C_ULONG, &Second, 0, &lSecond);

	// SQL문을 실행한다.
	if (SQLExecDirect(hStmt, (SQLCHAR*)"select * from Data", SQL_NTS) != SQL_SUCCESS) 
	{
		return FALSE;
	}

	SetBkMode(hMemDC, TRANSPARENT);
	font = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
		VARIABLE_PITCH | FF_ROMAN, "맑은 고딕");
	oldfont = (HFONT)SelectObject(hMemDC, font);
	SetTextColor(hMemDC, RGB(0, 0, 0));

	// 읽어온 데이터 출력
	while (SQLFetch(hStmt) != SQL_NO_DATA)
	{
		wsprintf(str, "%d위     %s     %d분 %d초", Num, ID, Minute, Second);
		SetRect(&rt, 100, 50 + y * 50, 700, 700);
		DrawText(hMemDC, str, -1, &rt, DT_WORDBREAK);
		y++;
	};

	DeleteObject(SelectObject(hMemDC, oldfont));
	ReleaseDC(hWndMain, hMemDC);

	if (hStmt) SQLCloseCursor(hStmt);
	return TRUE;
}

#pragma endregion

// 시간 기록
void Timer(HDC hMemDC)
{
	HFONT font, oldfont;
	RECT rt;
	TCHAR str[30];

	SetRect(&rt, 580, 25, 700, 100);

	wsprintf(str, "%d : %d", minute, second);

	SetBkMode(hMemDC, TRANSPARENT);
	font = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
		VARIABLE_PITCH | FF_ROMAN, "맑은 고딕");
	oldfont = (HFONT)SelectObject(hMemDC, font);
	
	SetTextColor(hMemDC, RGB(0, 0, 0));		// 글자 색
	DrawText(hMemDC, str, -1, &rt, DT_WORDBREAK);
	DeleteObject(SelectObject(hMemDC, oldfont));
}

// 버튼 고르기 함수
void SelectButton(int m)
{
	switch (m)
	{
	case UP:
		if (titleSelect == 2)
		{
			titleSelect = 1;
			PlaySound("Click.wav", NULL, SND_ASYNC);
		}
		break;
	case DOWN:
		if (titleSelect == 1)
		{
			titleSelect = 2;
			PlaySound("Click.wav", NULL, SND_ASYNC);
		}
		break;
	}
}

// 타일 변경 함수
void StageTile(HDC hMemDC)
{
	switch (stage)
	{
	case 1:
		for (int y = 0; y < 7; y++)
		{
			for (int x = 0; x < 7; x++)
			{
				map[y][x] = map1[y][x];
			}
		}
		break;
	case 2:
		for (int y = 0; y < 7; y++)
		{
			for (int x = 0; x < 7; x++)
			{
				map[y][x] = map2[y][x];
			}
		}
		break;
	case 3:
		for (int y = 0; y < 7; y++)
		{
			for (int x = 0; x < 7; x++)
			{
				map[y][x] = map3[y][x];
			}
		}
		break;
	case 4:
		for (int y = 0; y < 7; y++)
		{
			for (int x = 0; x < 7; x++)
			{
				map[y][x] = map4[y][x];
			}
		}
		break;
	case 5:
		for (int y = 0; y < 7; y++)
		{
			for (int x = 0; x < 7; x++)
			{
				map[y][x] = map5[y][x];
			}
		}
		break;
	case 6:
		for (int y = 0; y < 7; y++)
		{
			for (int x = 0; x < 7; x++)
			{
				map[y][x] = map6[y][x];
			}
		}
		break;
	}

	for (int y = 0; y < 7; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			TileBlt(hMemDC, (x * 72) + (y * 18), 150 + (y * 18), map[y][x]);
		}
	}
}

// 스테이지 클리어 체크
void ClearCheck()
{
	if (map[pArry][pArrx] == 0 && BoxState == 2)
	{
		pArry = 0;
		pArrx = 0;
		PlaySound("Clear.wav", NULL, SND_ASYNC);
		MessageBox(hWndMain, TEXT("클리어!"), TEXT("알림"), MB_OK);
		px = 0;
		py = 150;
		BoxState = 0;
		if (stage != 6)
		{
			stage++;
		}
		else
		{
			start = false;
		}
	}
}

// 타일 체크
bool TileCheck(int m)
{
	switch (m)
	{
	case UP:
		if (map[pArry - 1][pArrx] < 3)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case DOWN:
		if (map[pArry + 1][pArrx] < 3)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case LEFT:
		if (map[pArry][pArrx - 1] < 3)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case RIGHT:
		if (map[pArry][pArrx + 1] < 3)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	}
}

// 타일 그리기 함수
void TileBlt(HDC hMemDC, int x, int y, int m)
{
	if (m == 3)
	{
		return;
	}
	TransBlt(hMemDC, x, y, TileBit[m], RGB(255, 255, 255));
}

// 박스 프레임 관리
void BoxFrame()
{
	if (move)
	{
		if (pfrm < 6)
		{
			pfrm++;
		}
		else
		{
			move = false;
			pfrm = 0;
			ClearCheck();		// 박스가 착지했을 때 체크
		}
	}
}

// 키입력에 관한 함수입니다
void BoxMove(int m)
{
	switch (m)
	{
	case UP:
		// 0  1  2  3  4
		// 5  6  7  8  9
		// 10 11 12 13 14
		// 15 16 17 18 19
		if (pArry == 0 || !TileCheck(UP))
		{
			return;
		}
		pArry--;
		switch (BoxState)
		{
		case 0:
			motion = 19;
			BoxState = 1;
			break;
		case 1:
			motion = 15;
			BoxState = 2;
			break;
		case 2:
			motion = 16;
			BoxState = 3;
			break;
		case 3:
			motion = 17;
			BoxState = 0;
			break;
		case 5:
			motion = 15;
			break;
		default:
			motion = 18;
			break;
		}
		break;
	case RIGHT:
		if (pArrx == 6 || !TileCheck(RIGHT))
		{
			return;
		}
		pArrx++;
		switch (BoxState)
		{
		case 0:
			motion = 14;
			BoxState = 5;
			break;
		case 1:
			motion = 10;
			break;
		case 2:
			motion = 11;
			BoxState = 4;
			break;
		case 4:
			motion = 12;
			BoxState = 0;
			break;
		case 5:
			motion = 10;
			BoxState = 2;
			break;
		default:
			motion = 13;
			break;
		}
		break;
	case DOWN:
		if (pArry == 7 || !TileCheck(DOWN))
		{
			return;
		}
		pArry++;
		switch (BoxState)
		{
		case 0:
			motion = 2;
			BoxState = 3;
			break;
		case 1:
			motion = 1;
			BoxState = 0;
			break;
		case 2:
			motion = 0;
			BoxState = 1;
			break;
		case 3:
			motion = 4;
			BoxState = 2;
			break;
		case 5:
			motion = 0;
			break;
		default:
			motion = 3;
			break;
		}
		break;
	case LEFT:
		if (pArrx == 0 || !TileCheck(LEFT))
		{
			return;
		}
		pArrx--;
		switch (BoxState)
		{
		case 0:
			motion = 7;
			BoxState = 4;
			break;
		case 1:
			motion = 5;
			break;
		case 2:
			motion = 5;
			BoxState = 5;
			break;
		case 4:
			motion = 9;
			BoxState = 2;
			break;
		case 5:
			motion = 6;
			BoxState = 0;
			break;
		default:
			motion = 8;
			break;
		}
		break;
	}
	PlaySound("Move.wav", NULL, SND_ASYNC);
	move = true;
}

// 움직이는 그림을 그리는 함수입니다
void MoveTransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask, int frm, int imgSizeX, int imgSizeY)
{
	BITMAP bm;
	HDC hMemDC;
	POINT ptSize;

	HBITMAP hOldBitmap;

	hMemDC = CreateCompatibleDC(hdc);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;

	TransparentBlt(hdc, x, y, imgSizeX, imgSizeY, hMemDC, frm * imgSizeX, 0, imgSizeX, ptSize.y, clrMask);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
}

// 정해진 색을 지워서 그려주는 함수입니다
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask)
{
	BITMAP bm;
	HDC hMemDC;
	POINT ptSize;
	HBITMAP hOldBitmap;

	hMemDC = CreateCompatibleDC(hdc);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;

	TransparentBlt(hdc, x, y, ptSize.x, ptSize.y, hMemDC, 0, 0, ptSize.x, ptSize.y, clrMask);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
}

// 그림을 그려주는 함수입니다
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}

// 더블버퍼링의 핵심입니다
void OnTimer()
{
	RECT crt;
	HDC hdc, hMemDC;
	HBITMAP OldBit;

	GetClientRect(hWndMain, &crt);
	hdc = GetDC(hWndMain);

	if (!hBit)
	{
		hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	}

	hMemDC = CreateCompatibleDC(hdc);
	OldBit = (HBITMAP)SelectObject(hMemDC, hBit);

	if (stage == 6 && !start)
	{
		DrawBitmap(hMemDC, 0, 0, RankingBit);
		if (editcheck)
		{
			dbcheck = true;
			hEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
				ES_AUTOHSCROLL,
				300, 600, 100, 25, hWndMain, (HMENU)100, g_hInst, NULL);
			OldEditProc1 = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditSubProc1);
			editcheck = false;
		}
	}
	else if (stage == 0)
	{
		DrawBitmap(hMemDC, 0, 0, TitleBit);
		if (titleSelect == 1)
		{
			TransBlt(hMemDC, 400, 480, ArrowBit, RGB(255, 0, 255));
		}
		else
		{
			TransBlt(hMemDC, 400, 580, ArrowBit, RGB(255, 0, 255));
		}
	}
	else if (stage > 0)
	{
		DrawBitmap(hMemDC, 0, 0, BackgroundBit);		// 배경 그리기

		StageTile(hMemDC);

		if (move)
		{
			MoveTransBlt(hMemDC, px, py, BoxMoveBit[motion], RGB(255, 0, 255), pfrm, 150, 150);
			switch (motion)
			{
			case 0:		// 아래
			case 1:
			case 2:
			case 3:
			case 4:
				px += 3;
				py += 3;
				break;
			case 5:		// 왼
			case 6:
			case 7:
			case 8:
			case 9:
				px -= 12;
				break;
			case 10:		// 오른
			case 11:
			case 12:
			case 13:
			case 14:
				px += 12;
				break;
			case 15:		// 위
			case 16:
			case 17:
			case 18:
			case 19:
				px -= 3;
				py -= 3;
				break;
			}
		}
		else
		{
			switch (BoxState)
			{
			case 0:
				TransBlt(hMemDC, px, py, BoxBit[0], RGB(255, 0, 255));
				break;
			case 3:
				TransBlt(hMemDC, px, py, BoxBit[1], RGB(255, 0, 255));
				break;
			case 4:
				TransBlt(hMemDC, px, py, BoxBit[2], RGB(255, 0, 255));
				break;
			case 1:
			case 2:
			case 5:
				TransBlt(hMemDC, px, py, BoxBit[3], RGB(255, 0, 255));
				break;

			}
		}

		Timer(hMemDC);		// 시간
	}

	if (dbcheck)
	{
		DBExecuteSQL(hMemDC);
	}

	SelectObject(hMemDC, OldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWndMain, hdc);
	InvalidateRect(hWndMain, NULL, false);
}

// 시작 시
void Create()
{
	// 비트맵 로드
	for (int i = 0; i < 4; i++)
	{
		BoxBit[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP21 + i));			// 박스
	}
	for (int i = 0; i < 20; i++)
	{
		BoxMoveBit[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1 + i));		// 박스 이동모션
	}
	for (int i = 0; i < 3; i++)
	{
		TileBit[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP25 + i));		// 타일
	}
	BackgroundBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP28));		// 배경
	TitleBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP29));			// 타이틀
	ArrowBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP30));			// 화살표

	RankingBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP31));		// 랭킹표
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (iMessage) {
	case WM_CREATE:
		srand(unsigned(time));
		SetTimer(hWnd, 0, 60, NULL);
		SetTimer(hWnd, 1, 60, NULL);
		SetTimer(hWnd, 2, 1000, NULL);
		Create();

		BGMStart();

		// DB연결
		if (DBConnect() == FALSE)
		{
			MessageBox(hWndMain, "데이터 베이스에 연결할 수 없습니다", "에러", MB_OK);
			return -1;
		}
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
		case 0:
			OnTimer();
			break;
		case 1:
			BoxFrame();
			break;
		case 2:
			if (start)
			{
				if (second != 59)
				{
					second++;
				}
				else
				{
					second = 0;
					minute++;
				}
			}
			break;
		case 3:
			stage = 0;
			minute = 0;
			second = 0;
			dbcheck = false;
			editcheck = true;
			start = false;
			KillTimer(hWnd, 3);
			break;
		}
	case WM_KEYDOWN:
		if (stage == 0)
		{
			switch (wParam)
			{
			case VK_UP:
				SelectButton(UP);
				break;
			case VK_DOWN:
				SelectButton(DOWN);
				break;
			case VK_SPACE:
			case VK_RETURN:
				if (titleSelect == 1)
				{
					stage++;
					start = true;
					PlaySound("Select.wav", NULL, SND_ASYNC);
				} 
				if (titleSelect == 2)
				{
					PostQuitMessage(0);
				}
				break;
			}
		}
		else if (!move)
		{
			switch (wParam)
			{
			case VK_RIGHT:
				BoxMove(RIGHT);
				break;
			case VK_LEFT:
				BoxMove(LEFT);
				break;
			case VK_UP:
				BoxMove(UP);
				break;
			case VK_DOWN:
				BoxMove(DOWN);
				break;
			case 'A':
				stage++;
				break;
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		DrawBitmap(hdc, 0, 0, hBit);

		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		DBDisConnect();
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

// 서브클래스
LRESULT CALLBACK EditSubProc1(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	TCHAR szSQL[128];

	TCHAR str[128];
	SQLCHAR ID[30];

	switch (iMessage)
	{
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			GetWindowText(hWnd, (TCHAR*)ID, 30);		// 글을 가져오고
			SetWindowText(hWnd, TEXT(""));

			TCHAR szSQL[128];

			wsprintf(szSQL, "Insert into Data (Num, ID, Minute, Second) VALUES (%d, '%s', %d, %d)", 10, ID, minute, second);

			if (SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS) != SQL_SUCCESS)
			{
				MessageBox(hWndMain, "오류.", "오류", MB_OK);
			}

			SQLCloseCursor(hStmt);

			UpdateUser();

			SetTimer(hWndMain, 3, 5000, NULL);

			ShowWindow(hEdit, SW_HIDE);
		}
		break;
	}
	//SQLCloseCursor(hStmt);
	return CallWindowProc(OldEditProc1, hWnd, iMessage, wParam, lParam);
}