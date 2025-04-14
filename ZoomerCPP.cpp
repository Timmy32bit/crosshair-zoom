#include <windows.h>
#include <magnification.h>
#include <conio.h>
#include <iostream>

#pragma comment(lib, "Magnification.lib")

HWND hwnd_host = nullptr;
HWND hwnd_mag = nullptr;

const float MAG_FACTOR = 2.0f;

const int LENS_RADIUS = 200;

const int LENS_WIDTH = LENS_RADIUS;
const int LENS_HEIGHT = LENS_RADIUS;

const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
const int HOST_X = (SCREEN_WIDTH - LENS_RADIUS) / 2;
const int HOST_Y = (SCREEN_HEIGHT - LENS_RADIUS) / 2;

const int FPS = 60;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void update() {
    int w = LENS_WIDTH / (int)MAG_FACTOR;
    int h = LENS_HEIGHT / (int)MAG_FACTOR;

    RECT rect;
    rect.left = HOST_X + (LENS_WIDTH - w) / 2;
    rect.top = HOST_Y + (LENS_HEIGHT - h) / 2;
    rect.right = rect.left + w;
    rect.bottom = rect.top + h;

    MagSetWindowSource(hwnd_mag, rect);
    InvalidateRect(hwnd_mag, NULL, TRUE);
    UpdateWindow(hwnd_mag);
}

void cleanup() {
    DestroyWindow(hwnd_mag);
    DestroyWindow(hwnd_mag);
    MagUninitialize();
}

int main() {
    MagInitialize();

    HINSTANCE instance = GetModuleHandle(NULL);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"Zoomerrr";
    RegisterClass(&wc);

    hwnd_host = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        wc.lpszClassName,
        L"",
        WS_POPUP,
        HOST_X, HOST_Y,
        LENS_WIDTH, LENS_HEIGHT,
        NULL, NULL,
        instance,
        NULL
    );

    if (!hwnd_host)
        cleanup();

    SetLayeredWindowAttributes(hwnd_host, RGB(255, 0, 255), 0, LWA_COLORKEY);

    hwnd_mag = CreateWindowExW(
        0,
        WC_MAGNIFIER,
        L"Zoomerrr",
        WS_CHILD | WS_VISIBLE,
        0, 0, LENS_WIDTH, LENS_HEIGHT,
        hwnd_host,
        NULL,
        instance,
        NULL
    );

    if (!hwnd_mag)
        cleanup();

    MAGTRANSFORM matrix = { {
        { MAG_FACTOR, 0.0f, 0.0f },
        { 0.0f, MAG_FACTOR, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    } };

    MagSetWindowTransform(hwnd_mag, &matrix);

    ShowWindow(hwnd_host, SW_SHOW);
    SetWindowPos(hwnd_host, HWND_TOPMOST, HOST_X, HOST_Y, LENS_WIDTH, LENS_HEIGHT, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    std::cout << "Zooming baby! Press END to exit" << std::endl;
    std::cout << "> Press HOME to toggle" << std::endl;
    std::cout << "> Press END to exit" << std::endl;

    bool toggled = true;
    bool old_state = false;
    MSG msg;

    while (true) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
               cleanup();

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        if (GetAsyncKeyState(VK_END)) {
            cleanup();
            return 0;
        }

        update();

        if (GetAsyncKeyState(VK_HOME) && !old_state)
            toggled = !toggled;

        old_state = GetAsyncKeyState(VK_HOME);

        SetWindowPos(hwnd_host, HWND_TOPMOST, HOST_X, HOST_Y, LENS_WIDTH, LENS_HEIGHT, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

        ShowWindow(hwnd_host, toggled ? SW_SHOW : SW_HIDE);

        //if (_kbhit() && _getch() == 27)
        //    break;

        Sleep(static_cast<int>(1000.0 / FPS));
    }

    return 0;
}
