#include <windows.h>
#include "window.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CalendarWindow window;
    if (!window.Create(hInstance)) {
        MessageBox(NULL, L"创建窗口失败", L"错误", MB_ICONERROR);
        return -1;
    }
    window.Show(nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
