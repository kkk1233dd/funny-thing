#include "utils.h"
#include <sstream>
#include <iomanip>
#include <commctrl.h>

namespace utils {

struct InputBoxData {
    const wchar_t* prompt;
    const wchar_t* title;
    wchar_t* buffer;
    int bufferSize;
    HWND hEdit;
    bool result;
};

static INT_PTR CALLBACK InputBoxDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    InputBoxData* data = (InputBoxData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg) {
        case WM_INITDIALOG: {
            data = (InputBoxData*)lParam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
            SetWindowText(hwnd, data->title);

            HWND hPrompt = GetDlgItem(hwnd, 100);
            SetWindowText(hPrompt, data->prompt);

            HWND hEdit = GetDlgItem(hwnd, 101);
            data->hEdit = hEdit;
            SetWindowText(hEdit, data->buffer);
            SetFocus(hEdit);
            return FALSE;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                if (data && data->hEdit) {
                    GetWindowText(data->hEdit, data->buffer, data->bufferSize);
                    data->result = true;
                }
                EndDialog(hwnd, IDOK);
                return TRUE;
            } else if (LOWORD(wParam) == IDCANCEL) {
                if (data) data->result = false;
                EndDialog(hwnd, IDCANCEL);
                return TRUE;
            }
            break;

        case WM_CLOSE:
            if (data) data->result = false;
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
    }

    return FALSE;
}

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    std::wstring result(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], len);
    return result;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

std::wstring IntToWStr(int value) {
    return std::to_wstring(value);
}

int WStrToInt(const std::wstring& str) {
    try {
        return std::stoi(str);
    } catch (...) {
        return 0;
    }
}

std::wstring GetCurrentDateStr() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[32];
    swprintf_s(buf, 32, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    return std::wstring(buf);
}

std::wstring FormatDateMD(const std::wstring& dateStr) {
    if (dateStr.size() < 10) return dateStr;
    int year = WStrToInt(dateStr.substr(0, 4));
    int month = WStrToInt(dateStr.substr(5, 2));
    int day = WStrToInt(dateStr.substr(8, 2));
    wchar_t buf[32];
    swprintf_s(buf, 32, L"%d月%d日", month, day);
    return std::wstring(buf);
}

void DrawRoundRect(HDC hdc, int left, int top, int right, int bottom, int radius) {
    int x1 = left;
    int y1 = top;
    int x2 = right;
    int y2 = bottom;
    int d = radius * 2;

    BeginPath(hdc);
    MoveToEx(hdc, x1 + radius, y1, NULL);
    LineTo(hdc, x2 - radius, y1);
    AngleArc(hdc, x2 - radius, y1 + radius, radius, 900, -900);
    LineTo(hdc, x2, y2 - radius);
    AngleArc(hdc, x2 - radius, y2 - radius, radius, 0, -900);
    LineTo(hdc, x1 + radius, y2);
    AngleArc(hdc, x1 + radius, y2 - radius, radius, 2700, -900);
    LineTo(hdc, x1, y1 + radius);
    AngleArc(hdc, x1 + radius, y1 + radius, radius, 1800, -900);
    EndPath(hdc);
    StrokePath(hdc);
}

void FillRoundRect(HDC hdc, int left, int top, int right, int bottom, int radius, COLORREF color) {
    int x1 = left;
    int y1 = top;
    int x2 = right;
    int y2 = bottom;
    int d = radius * 2;

    HBRUSH hBrush = CreateSolidBrush(color);
    HPEN hPen = CreatePen(PS_NULL, 0, color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    BeginPath(hdc);
    MoveToEx(hdc, x1 + radius, y1, NULL);
    LineTo(hdc, x2 - radius, y1);
    AngleArc(hdc, x2 - radius, y1 + radius, radius, 900, -900);
    LineTo(hdc, x2, y2 - radius);
    AngleArc(hdc, x2 - radius, y2 - radius, radius, 0, -900);
    LineTo(hdc, x1 + radius, y2);
    AngleArc(hdc, x1 + radius, y2 - radius, radius, 2700, -900);
    LineTo(hdc, x1, y1 + radius);
    AngleArc(hdc, x1 + radius, y1 + radius, radius, 1800, -900);
    EndPath(hdc);
    FillPath(hdc);

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

void DrawTextCentered(HDC hdc, const std::wstring& text, RECT rect, COLORREF color, HFONT font) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    HFONT hOldFont = (HFONT)SelectObject(hdc, font);
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
}

void DrawTextLeft(HDC hdc, const std::wstring& text, RECT rect, COLORREF color, HFONT font) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    HFONT hOldFont = (HFONT)SelectObject(hdc, font);
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    SelectObject(hdc, hOldFont);
}

HFONT CreateFontSimple(int height, int weight, const wchar_t* faceName, bool italic) {
    return CreateFontW(
        height, 0, 0, 0, weight,
        italic ? TRUE : FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        faceName
    );
}

COLORREF ARGBToCOLORREF(BYTE a, BYTE r, BYTE g, BYTE b) {
    return RGB(r, g, b);
}

int InputBoxW(HWND hwndParent, const wchar_t* prompt, const wchar_t* title, wchar_t* buffer, int bufferSize) {
    InputBoxData data;
    data.prompt = prompt;
    data.title = title;
    data.buffer = buffer;
    data.bufferSize = bufferSize;
    data.hEdit = NULL;
    data.result = false;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefDlgProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"InputBoxTempClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"InputBoxTempClass",
        title,
        WS_DLGFRAME | WS_SYSMENU | DS_CENTER,
        CW_USEDEFAULT, CW_USEDEFAULT,
        320, 160,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );

    if (!hwnd) return 0;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&data);
    SetWindowText(hwnd, title);

    CreateWindowW(L"STATIC", prompt, WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 20, 260, 25, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);

    HWND hEdit = CreateWindowW(L"EDIT", buffer,
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        20, 50, 260, 28, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
    data.hEdit = hEdit;

    CreateWindowW(L"BUTTON", L"确定",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        110, 90, 70, 28, hwnd, (HMENU)IDOK, GetModuleHandle(NULL), NULL);

    CreateWindowW(L"BUTTON", L"取消",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 90, 70, 28, hwnd, (HMENU)IDCANCEL, GetModuleHandle(NULL), NULL);

    SendMessage(hwnd, WM_INITDIALOG, 0, (LPARAM)&data);

    RECT rc;
    GetWindowRect(hwnd, &rc);
    SetWindowPos(hwnd, HWND_TOPMOST, rc.left, rc.top, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShowWindow(hwnd, SW_SHOW);
    SetFocus(hEdit);
    SendMessage(hEdit, EM_SETSEL, 0, -1);

    MSG msg;
    BOOL ret;
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) break;
        if (msg.hwnd == hwnd || IsChild(hwnd, msg.hwnd)) {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                data.result = false;
                break;
            }
            if (msg.message == WM_COMMAND) {
                if (LOWORD(msg.wParam) == IDOK) {
                    GetWindowText(hEdit, buffer, bufferSize);
                    data.result = true;
                    break;
                }
                if (LOWORD(msg.wParam) == IDCANCEL) {
                    data.result = false;
                    break;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hwnd);
    UnregisterClassW(L"InputBoxTempClass", GetModuleHandle(NULL));

    return data.result ? (int)wcslen(buffer) : 0;
}

}
