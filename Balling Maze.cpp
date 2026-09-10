// Balling Maze.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "Balling Maze.h"
#include "Map.h"
#include "Player.h"
#include "Audio.h"
#include "TitleScreen.h"
#include <windowsx.h>
#include <cmath>
#include <cctype>
#include <cstring>
#include <GL/gl.h>
#include <GL/glu.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
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
static float g_thirdPersonDist = 6.0f;
static HDC g_hDC = NULL;
static HGLRC g_hGLRC = NULL;
static TitleScreen g_titleScreen;
static bool g_titleActive = true;
static bool g_gameOverActive = false;
static bool g_gameOverSoundPlayed = false;
static unsigned int g_uiFontBase = 0;
static GLUquadric* g_sphereQuadric = nullptr;

static void ResetGameState()
{
    g_map.loadFromFile("assets\\maps\\map1.txt");
    auto it = g_map.points.find('S');
    if (it != g_map.points.end() && !it->second.empty()) {
        auto p = it->second.front();
        g_player.x = (float)p.second + 0.5f;
        g_player.z = (float)p.first + 0.5f;
        g_player.y = 0.0f;
        g_player.vy = 0.0f;
        g_player.onGround = true;
        g_player.sphere.setPosition(g_player.x, g_player.y, g_player.z);
    }
    std::memset(g_player.keys, 0, sizeof(g_player.keys));
    g_enemies.clear();
    g_keyCollected.clear();
    g_gameOverActive = false;
    g_gameOverSoundPlayed = false;
}

static void EnsureUiFont(HDC hdc)
{
    if (g_uiFontBase != 0) return;
    g_uiFontBase = glGenLists(96);
    HFONT font = CreateFontA(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, "Segoe UI");
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
    wglUseFontBitmapsA(hdc, 32, 96, g_uiFontBase);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

static void DrawUiText(const char* text, float x, float y, float scale)
{
    if (g_uiFontBase == 0) return;
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glRasterPos2f(0.0f, 0.0f);
    glListBase(g_uiFontBase - 32);
    glCallLists((GLsizei)std::strlen(text), GL_UNSIGNED_BYTE, text);
    glPopMatrix();
}

static void DrawGameOverOverlay(HDC hdc, int width, int height)
{
    EnsureUiFont(hdc);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.02f, 0.03f, 0.04f, 0.78f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)width, 0.0f);
    glVertex2f((float)width, (float)height);
    glVertex2f(0.0f, (float)height);
    glEnd();

    float panelWidth = width < 560 ? (float)width * 0.86f : 480.0f;
    float panelLeft = ((float)width - panelWidth) * 0.5f;
    float panelTop = (float)height * 0.34f;
    glColor4f(0.90f, 0.70f, 0.25f, 0.96f);
    glBegin(GL_QUADS);
    glVertex2f(panelLeft, panelTop);
    glVertex2f(panelLeft + panelWidth, panelTop);
    glVertex2f(panelLeft + panelWidth, panelTop + 180.0f);
    glVertex2f(panelLeft, panelTop + 180.0f);
    glEnd();

    glColor3f(0.05f, 0.06f, 0.07f);
    DrawUiText("GAME OVER", panelLeft + 118.0f, panelTop + 64.0f, 1.4f);
    DrawUiText("ENTER: RETRY", panelLeft + 136.0f, panelTop + 118.0f, 0.9f);
    DrawUiText("ESC: EXIT", panelLeft + 166.0f, panelTop + 152.0f, 0.9f);

    glDisable(GL_BLEND);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

static void DrawSphereEntity(const SphereEntity& sphere, float red, float green, float blue)
{
    if (!g_sphereQuadric) {
        g_sphereQuadric = gluNewQuadric();
        gluQuadricNormals(g_sphereQuadric, GLU_SMOOTH);
    }

    glPushMatrix();
    sphere.applyOpenGLTransform();
    glColor3f(red, green, blue);
    gluQuadricDrawStyle(g_sphereQuadric, GLU_FILL);
    gluSphere(g_sphereQuadric, sphere.radius, 24, 16);
    glColor3f(0.96f, 0.96f, 0.88f);
    gluQuadricDrawStyle(g_sphereQuadric, GLU_LINE);
    gluSphere(g_sphereQuadric, sphere.radius * 1.015f, 16, 10);
    gluQuadricDrawStyle(g_sphereQuadric, GLU_FILL);
    glPopMatrix();
}

static void ResolveEnemySphereCollisions()
{
    for (size_t i = 0; i < g_enemies.size(); ++i) {
        for (size_t j = i + 1; j < g_enemies.size(); ++j) {
            Enemy& first = g_enemies[i];
            Enemy& second = g_enemies[j];
            float dx = second.x - first.x;
            float dz = second.z - first.z;
            float dist = sqrtf(dx * dx + dz * dz);
            float minDist = first.sphere.radius + second.sphere.radius;
            if (dist <= 0.001f || dist >= minDist) continue;

            float push = (minDist - dist) * 0.5f;
            float nx = dx / dist;
            float nz = dz / dist;
            first.x -= nx * push;
            first.z -= nz * push;
            second.x += nx * push;
            second.z += nz * push;
            first.sphere.setPosition(first.x, first.y, first.z);
            second.sphere.setPosition(second.x, second.y, second.z);
        }
    }
}

static void HandleTitleAction(HWND hWnd, TitleAction action)
{
    if (action == TitleAction::Start) {
        g_titleActive = false;
        Audio::playBGM("assets/sounds/maou_bgm_cyber41.mp3", 0.8f);
        InvalidateRect(hWnd, NULL, FALSE);
    } else if (action == TitleAction::Exit) {
        DestroyWindow(hWnd);
    }
}

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
       
   } else {
       // set player start if S exists
       auto it = g_map.points.find('S');
       if (it != g_map.points.end() && !it->second.empty()) {
           auto p = it->second.front(); // row,col
           g_player.x = (float)p.second + 0.5f;
           g_player.z = (float)p.first + 0.5f;
           g_player.y = 0.0f;
           g_player.onGround = true;
           g_player.sphere.setPosition(g_player.x, g_player.y, g_player.z);
       }
   }

   // load settings if present
   g_settings.load("settings.json");

   // initialize OpenGL for this window
   g_hDC = GetDC(hWnd);
   PIXELFORMATDESCRIPTOR pfd = {};
   pfd.nSize = sizeof(pfd);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 24;
   pfd.cDepthBits = 16;
   int pf = ChoosePixelFormat(g_hDC, &pfd);
   if (pf == 0 || !SetPixelFormat(g_hDC, pf, &pfd)) {
    
   } else {
       g_hGLRC = wglCreateContext(g_hDC);
       if (!g_hGLRC || !wglMakeCurrent(g_hDC, g_hGLRC)) {
        
       } else {
           // basic GL setup
           glClearColor(0.5f, 0.8f, 0.95f, 1.0f);
           glEnable(GL_DEPTH_TEST);
       }
   }

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
            // render OpenGL scene if available
            if (g_hDC && g_hGLRC) {
                // make sure context is current
                wglMakeCurrent(g_hDC, g_hGLRC);
                // set viewport
                RECT rc; GetClientRect(hWnd, &rc);
                int w = rc.right - rc.left; int h = rc.bottom - rc.top;
                glViewport(0,0,w,h);

                if (g_titleActive) {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    g_titleScreen.draw(g_hDC, w, h);
                    SwapBuffers(g_hDC);
                    wglMakeCurrent(NULL, NULL);
                    EndPaint(hWnd, &ps);
                    break;
                }

                // 3D perspective projection
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                double aspect = (h==0)?1.0:((double)w/(double)h);
                gluPerspective(60.0, aspect, 0.1, 100.0);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                // compute camera
                const double PI = 3.14159265358979323846;
                double yawRad = g_cameraYaw * PI / 180.0;
                double pitchRad = g_cameraPitch * PI / 180.0;
                // forward vector
                double fx = cos(pitchRad) * cos(yawRad);
                double fy = sin(pitchRad);
                double fz = cos(pitchRad) * sin(yawRad);

                double camX, camY, camZ, lookX, lookY, lookZ;
                if (g_cameraFirstPerson) {
                    camX = g_player.x;
                    camY = 0.6 + g_player.y; // eye height
                    camZ = g_player.z;
                    lookX = camX + fx;
                    lookY = camY + fy;
                    lookZ = camZ + fz;
                } else {
                    double dist = (double)g_thirdPersonDist;
                    double above = 3.0;
                    camX = g_player.x - fx * dist;
                    camY = g_player.y + above;
                    camZ = g_player.z - fz * dist;
                    lookX = g_player.x;
                    lookY = g_player.y + 0.5;
                    lookZ = g_player.z;
                }

                // set camera
                gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0.0, 1.0, 0.0);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // helper to draw cube centered at cx,cy,cz with size s
                auto drawCube = [](float cx, float cy, float cz, float s){
                    float h = s*0.5f;
                    float x0 = cx - h, x1 = cx + h;
                    float y0 = cy - h, y1 = cy + h;
                    float z0 = cz - h, z1 = cz + h;
                    glBegin(GL_QUADS);
                    // top
                    glVertex3f(x0,y1,z0); glVertex3f(x1,y1,z0); glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
                    // bottom
                    glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1); glVertex3f(x0,y0,z1);
                    // front
                    glVertex3f(x0,y0,z1); glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
                    // back
                    glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0); glVertex3f(x1,y1,z0); glVertex3f(x0,y1,z0);
                    // left
                    glVertex3f(x0,y0,z0); glVertex3f(x0,y0,z1); glVertex3f(x0,y1,z1); glVertex3f(x0,y1,z0);
                    // right
                    glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1); glVertex3f(x1,y1,z0);
                    glEnd();
                };

                // draw floor (pure white)
                glColor3f(1.0f, 1.0f, 1.0f);
                glBegin(GL_QUADS);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f((float)g_map.width, 0.0f, 0.0f);
                glVertex3f((float)g_map.width, 0.0f, (float)g_map.height);
                glVertex3f(0.0f, 0.0f, (float)g_map.height);
                glEnd();

                // draw walls as cubes (height 1)
                for (int r=0;r<g_map.height;++r) {
                    for (int c=0;c<g_map.grid[r].size();++c) {
                        char ch = g_map.grid[r][c];
                        if (ch == '#') {
                            glColor3f(0.6f,0.6f,0.6f);
                            drawCube((float)c+0.5f, 0.5f, (float)r+0.5f, 1.0f);
                        } else if (ch == 'G') {
                            glColor3f(0.9f,0.8f,0.2f);
                            drawCube((float)c+0.5f, 0.5f, (float)r+0.5f, 1.0f);
                        }
                    }
                }

                // draw player as rolling sphere (紺色)
                DrawSphereEntity(g_player.sphere, 0.0f, 0.0f, 0.55f);

                // draw enemies (赤)
                for (auto &e : g_enemies) {
                    DrawSphereEntity(e.sphere, 1.0f, 0.0f, 0.0f);
                }

                if (g_gameOverActive) {
                    DrawGameOverOverlay(g_hDC, w, h);
                }

                SwapBuffers(g_hDC);
                wglMakeCurrent(NULL, NULL);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_KEYDOWN:
        {
            int vk = (int)wParam;
            if (g_titleActive) {
                HandleTitleAction(hWnd, g_titleScreen.onKeyDown(wParam));
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
            if (g_gameOverActive) {
                if (vk == VK_RETURN) {
                    ResetGameState();
                    Audio::playBGM("assets/sounds/maou_bgm_cyber41.mp3", 0.8f);
                    InvalidateRect(hWnd, NULL, FALSE);
                } else if (vk == VK_ESCAPE) {
                    DestroyWindow(hWnd);
                }
                break;
            }
            if (vk == 'W' || vk == 'A' || vk == 'S' || vk == 'D') {
                g_player.keys[vk] = true;
            }
            if (vk == VK_ESCAPE) {
                // pause / exit
             
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
        {
            if (g_titleActive) {
                RECT rc; GetClientRect(hWnd, &rc);
                g_titleScreen.onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), rc.right - rc.left, rc.bottom - rc.top);
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            if (g_titleActive) {
                RECT rc; GetClientRect(hWnd, &rc);
                HandleTitleAction(hWnd, g_titleScreen.onMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), rc.right - rc.left, rc.bottom - rc.top));
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
        }
        break;
    case WM_RBUTTONDOWN:
        {
            g_rightMouseDown = true;
            g_lastMouseX = GET_X_LPARAM(lParam);
            g_lastMouseY = GET_Y_LPARAM(lParam);
            SetCapture(hWnd);
            // hide cursor while rotating
            ShowCursor(FALSE);
        }
        break;
    case WM_RBUTTONUP:
        {
            g_rightMouseDown = false;
            ReleaseCapture();
            // restore cursor
            ShowCursor(TRUE);
        }
        break;
    case WM_MOUSEMOVE:
        {
            if (g_titleActive) {
                RECT rc; GetClientRect(hWnd, &rc);
                g_titleScreen.onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), rc.right - rc.left, rc.bottom - rc.top);
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
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
                // request redraw so view updates immediately
                InvalidateRect(hWnd, NULL, FALSE);
            }
        }
        break;
    case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            // decrease distance on wheel up
            g_thirdPersonDist -= (float)delta / 120.0f; // one notch = 1.0
            if (g_thirdPersonDist < 2.0f) g_thirdPersonDist = 2.0f;
            if (g_thirdPersonDist > 15.0f) g_thirdPersonDist = 15.0f;
            wchar_t buf[128];
            swprintf_s(buf, L"Third person distance: %.1f", g_thirdPersonDist);
            SetWindowTextW(hWnd, buf);
        }
        break;
    case WM_TIMER:
        {
            if (g_titleActive) {
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
            if (g_gameOverActive) {
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
            // update real player with global map
            float dt = 0.016f;
            g_player.update(dt, g_map, g_cameraYaw);

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
                            e.sphere.setPosition(e.x, e.y, e.z);
                            g_enemies.push_back(e);
                            // notify
                            wchar_t msg[128];
                            swprintf_s(msg, L"Key '%c' collected - enemy spawned", ch);
                            // show small message in title (will be overwritten next frame)
                            SetWindowTextW(hWnd, msg);
                            // play small SE (stub)
                            Audio::playSE("assets/sounds/maou_se_battle05.mp3");
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
                if (dist < g_player.sphere.radius + e.sphere.radius) {
                    g_gameOverActive = true;
                    std::memset(g_player.keys, 0, sizeof(g_player.keys));
                    if (!g_gameOverSoundPlayed) {
                        Audio::playSE("assets/sounds/maou_se_magical31.mp3");
                        g_gameOverSoundPlayed = true;
                    }
                    break;
                }
            }
            if (!g_gameOverActive) {
                ResolveEnemySphereCollisions();
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
            // request redraw to update the 3D view after game state changes
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_DESTROY:
        // cleanup OpenGL
        g_titleScreen.releaseGraphics();
        if (g_sphereQuadric) { gluDeleteQuadric(g_sphereQuadric); g_sphereQuadric = nullptr; }
        if (g_uiFontBase != 0) { glDeleteLists(g_uiFontBase, 96); g_uiFontBase = 0; }
        if (g_hGLRC) { wglMakeCurrent(NULL, NULL); wglDeleteContext(g_hGLRC); g_hGLRC = NULL; }
        if (g_hDC) { ReleaseDC(hWnd, g_hDC); g_hDC = NULL; }
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
