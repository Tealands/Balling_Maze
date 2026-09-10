// Balling Maze.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "Balling Maze.h"
#include "Map.h"
#include "Player.h"
#include "Audio.h"
#include <windowsx.h>
#include <cmath>
#include <cctype>
#include "Enemy.h"
#include <vector>
#include <map>
#include "Settings.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// Game state globals
static Map g_map;
static Player g_player;
static bool g_cameraFirstPerson = false;
static int g_panSign = 1; // flip when switching first/third
static bool g_rightMouseDown = false;
static int g_lastMouseX = 0, g_lastMouseY = 0;
static float g_cameraYaw = 0.0f, g_cameraPitch = 0.0f;
static std::vector<Enemy> g_enemies;
static std::map<char,bool> g_keyCollected;
static Settings g_settings;

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BALLINGMAZE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BALLINGMAZE));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BALLINGMAZE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BALLINGMAZE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // --- マップとプレイヤー初期化 ---
   bool ok = g_map.loadFromFile("assets\\maps\\map1.txt");
   if (!ok) {
       MessageBoxW(hWnd, L"assets\\maps\\map1.txt の読み込みに失敗しました", L"Map Load Error", MB_OK | MB_ICONERROR);
   } else {
       // set player start if S exists
       auto it = g_map.points.find('S');
       if (it != g_map.points.end() && !it->second.empty()) {
           auto p = it->second.front(); // row,col
           g_player.x = (float)p.second + 0.5f;
           g_player.z = (float)p.first + 0.5f;
           g_player.y = 0.0f;
           g_player.onGround = true;
       }
   }

   // load settings if present
   g_settings.load("settings.json");

   // frame timer (~60fps)
   SetTimer(hWnd, 1, 16, NULL);

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // use global game state variables (g_map, g_player, g_cameraFirstPerson...)

    switch (message)
    {
    case WM_CREATE:
        // nothing for now
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            // TODO: 描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_KEYDOWN:
        {
            int vk = (int)wParam;
            if (vk == 'W' || vk == 'A' || vk == 'S' || vk == 'D') {
                g_player.keys[vk] = true;
            }
            if (vk == VK_ESCAPE) {
                // pause / exit
                MessageBoxW(hWnd, L"Paused (ESC pressed)", L"Pause", MB_OK);
            }
            if (vk == 'Q') {
                // toggle camera mode
                g_cameraFirstPerson = !g_cameraFirstPerson;
                g_panSign = g_cameraFirstPerson ? -1 : 1;
                std::wstring txt = g_cameraFirstPerson ? L"Camera: First Person" : L"Camera: Third Person";
                SetWindowTextW(hWnd, txt.c_str());
            }
        }
        break;
    case WM_KEYUP:
        {
            int vk = (int)wParam;
            if (vk == 'W' || vk == 'A' || vk == 'S' || vk == 'D') {
                g_player.keys[vk] = false;
            }
            if (vk == VK_SPACE) {
                // on key up do nothing
            }
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        break;
    case WM_RBUTTONDOWN:
        {
            g_rightMouseDown = true;
            g_lastMouseX = GET_X_LPARAM(lParam);
            g_lastMouseY = GET_Y_LPARAM(lParam);
            SetCapture(hWnd);
        }
        break;
    case WM_RBUTTONUP:
        {
            g_rightMouseDown = false;
            ReleaseCapture();
        }
        break;
    case WM_MOUSEMOVE:
        {
            // handle mouse pan when RMB is down
            if (g_rightMouseDown) {
                int mx = GET_X_LPARAM(lParam);
                int my = GET_Y_LPARAM(lParam);
                int dx = mx - g_lastMouseX; int dy = my - g_lastMouseY;
                g_lastMouseX = mx; g_lastMouseY = my;
                float sens = 0.25f; // degrees per pixel
                g_cameraYaw += dx * sens * (float)g_panSign;
                g_cameraPitch += dy * sens;
                if (g_cameraPitch > 89.0f) g_cameraPitch = 89.0f;
                if (g_cameraPitch < -89.0f) g_cameraPitch = -89.0f;
                // update title to show camera yaw/pitch
                wchar_t buf[128];
                swprintf_s(buf, L"Cam yaw=%.1f pitch=%.1f", g_cameraYaw, g_cameraPitch);
                SetWindowTextW(hWnd, buf);
            }
        }
        break;
    case WM_TIMER:
        {
            // update real player with global map
            float dt = 0.016f;
            g_player.update(dt, g_map);

            // check key pickup at player's cell
            int prow = (int)std::floor(g_player.z);
            int pcol = (int)std::floor(g_player.x);
            if (prow >= 0 && pcol >= 0 && prow < g_map.height && pcol < g_map.width) {
                if (pcol < (int)g_map.grid[prow].size()) {
                    char ch = g_map.grid[prow][pcol];
                    if ((ch == 'a' || ch == 'b' || ch == 'c') && !g_keyCollected[ch]) {
                        g_keyCollected[ch] = true;
                        // remove key from grid
                        g_map.grid[prow][pcol] = '.';
                        // spawn corresponding enemy at uppercase marker
                        char up = (char)toupper(ch);
                        auto it = g_map.points.find(up);
                        if (it != g_map.points.end() && !it->second.empty()) {
                            auto pos = it->second.front();
                            Enemy e; e.x = (float)pos.second + 0.5f; e.z = (float)pos.first + 0.5f; e.y = 0.0f; e.speed = 1.5f;
                            g_enemies.push_back(e);
                            // notify
                            wchar_t msg[128];
                            swprintf_s(msg, L"Key '%c' collected - enemy spawned", ch);
                            // show small message in title (will be overwritten next frame)
                            SetWindowTextW(hWnd, msg);
                            // play small SE (stub)
                            Audio::playSE("assets/maou_se_battle05.mp3");
                        }
                    }
                }
            }

            // check goal
            if (prow >= 0 && pcol >= 0 && prow < g_map.height && pcol < g_map.width) {
                if (pcol < (int)g_map.grid[prow].size()) {
                    char chg = g_map.grid[prow][pcol];
                    if (chg == 'G') {
                        int keysGot = 0;
                        if (g_keyCollected['a']) ++keysGot;
                        if (g_keyCollected['b']) ++keysGot;
                        if (g_keyCollected['c']) ++keysGot;
                        if (keysGot >= 3) {
                            // win
                            MessageBoxW(hWnd, L"You cleared the maze!", L"Clear", MB_OK | MB_ICONINFORMATION);
                            // record
                            g_settings.recordWin("map1");
                            g_settings.save("settings.json");
                            // reset state for now
                            g_enemies.clear();
                            g_keyCollected.clear();
                            // optionally advance to next map: try load map2
                            if (g_map.points.find('G') != g_map.points.end()) {
                                // simple: reload same map to reset
                                g_map.loadFromFile("assets\\maps\\map1.txt");
                            }
                        }
                    }
                }
            }

            // update enemies
            for (auto &e : g_enemies) {
                e.update(dt, g_map, g_player);
                float dx = e.x - g_player.x; float dz = e.z - g_player.z;
                float dist = sqrtf(dx*dx + dz*dz);
                if (dist < 0.5f) {
                    MessageBoxW(hWnd, L"Caught by enemy! Game Over.", L"Game Over", MB_OK | MB_ICONEXCLAMATION);
                    // play game over SE (stub)
                    Audio::playSE("assets/maou_se_magical31.mp3");
                    // reset player to start
                    auto it = g_map.points.find('S');
                    if (it != g_map.points.end() && !it->second.empty()) {
                        auto p = it->second.front();
                        g_player.x = (float)p.second + 0.5f; g_player.z = (float)p.first + 0.5f; g_player.y = 0.0f; g_player.onGround = true;
                    }
                    g_enemies.clear();
                    g_keyCollected.clear();
                    break;
                }
            }

            // build status string
            int keysGot = 0;
            if (g_keyCollected['a']) ++keysGot;
            if (g_keyCollected['b']) ++keysGot;
            if (g_keyCollected['c']) ++keysGot;
            wchar_t buf[256];
            swprintf_s(buf, L"Pos(%.2f,%.2f,%.2f) Keys:%d/3 Enemies:%d Cam:%s",
                g_player.x, g_player.y, g_player.z, keysGot, (int)g_enemies.size(), g_cameraFirstPerson ? L"1st" : L"3rd");
            SetWindowTextW(hWnd, buf);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
