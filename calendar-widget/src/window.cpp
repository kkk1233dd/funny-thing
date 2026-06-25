#include "window.h"
#include <fstream>
#include <algorithm>
#include <time.h>

ThemeColors g_themes[4];

void InitThemes() {
    // 春 - 樱落时节
    g_themes[0].bgPrimary = RGB(245, 250, 245);
    g_themes[0].bgSecondary = RGB(225, 240, 225);
    g_themes[0].accent = RGB(152, 210, 152);
    g_themes[0].accentDark = RGB(107, 172, 107);
    g_themes[0].textPrimary = RGB(46, 125, 50);
    g_themes[0].textSecondary = RGB(85, 139, 47);
    g_themes[0].seasonText = L"樱落时节";

    // 夏 - 蝉鸣之夏
    g_themes[1].bgPrimary = RGB(232, 245, 233);
    g_themes[1].bgSecondary = RGB(200, 230, 201);
    g_themes[1].accent = RGB(129, 199, 132);
    g_themes[1].accentDark = RGB(102, 187, 106);
    g_themes[1].textPrimary = RGB(46, 125, 50);
    g_themes[1].textSecondary = RGB(85, 139, 47);
    g_themes[1].seasonText = L"蝉鸣之夏";

    // 秋 - 枫红秋意
    g_themes[2].bgPrimary = RGB(255, 243, 224);
    g_themes[2].bgSecondary = RGB(255, 224, 178);
    g_themes[2].accent = RGB(255, 167, 38);
    g_themes[2].accentDark = RGB(255, 143, 0);
    g_themes[2].textPrimary = RGB(191, 93, 0);
    g_themes[2].textSecondary = RGB(230, 109, 0);
    g_themes[2].seasonText = L"枫红秋意";

    // 冬 - 雪落冬安
    g_themes[3].bgPrimary = RGB(227, 242, 253);
    g_themes[3].bgSecondary = RGB(187, 222, 251);
    g_themes[3].accent = RGB(100, 181, 246);
    g_themes[3].accentDark = RGB(66, 165, 245);
    g_themes[3].textPrimary = RGB(25, 118, 210);
    g_themes[3].textSecondary = RGB(57, 139, 247);
    g_themes[3].seasonText = L"雪落冬安";
}

ThemeColors& CurrentTheme() {
    return g_themes[0];
}

CalendarWindow::CalendarWindow()
    : hwnd_(NULL)
    , hInstance_(NULL)
    , isDragging_(false)
    , isCollapsed_(false)
    , themeIndex_(1)
{
    InitThemes();
}

CalendarWindow::~CalendarWindow() {
    if (hwnd_) DestroyWindow(hwnd_);
}

static CalendarWindow* g_window = NULL;

static LRESULT CALLBACK InputDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            std::wstring* prompt = (std::wstring*)lParam;
            SetDlgItemText(hDlg, 100, prompt->c_str());
            SendDlgItemMessage(hDlg, 101, EM_SETLIMITTEXT, 255, 0);
            SetFocus(GetDlgItem(hDlg, 101));
            return FALSE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                EndDialog(hDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDCANCEL);
                return TRUE;
            }
            break;
        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

bool CalendarWindow::ShowInputDialog(const wchar_t* title, const wchar_t* prompt, std::wstring& result) {
    std::wstring promptStr = prompt;

    HWND hDlg = CreateDialogParam(hInstance_, MAKEINTRESOURCE(0), hwnd_, InputDlgProc, (LPARAM)&promptStr);
    if (!hDlg) {
        wchar_t buf[256] = {0};
        wcscpy_s(buf, result.c_str());
        if (IDOK == MessageBox(hwnd_, prompt, title, MB_OKCANCEL | MB_ICONQUESTION)) {
            return true;
        }
        return false;
    }

    SetWindowText(hDlg, title);

    RECT rc;
    GetWindowRect(hwnd_, &rc);
    int dlgW = 300, dlgH = 140;
    int x = rc.left + (WINDOW_WIDTH - dlgW) / 2;
    int y = rc.top + 100;
    SetWindowPos(hDlg, NULL, x, y, dlgW, dlgH, SWP_NOZORDER);

    HDC hdc = GetDC(hDlg);

    RECT textRect = {20, 15, 260, 35};
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0,0,0));
    DrawText(hdc, prompt, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    HWND hEdit = CreateWindow(L"EDIT", result.c_str(),
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        20, 45, 260, 28, hDlg, (HMENU)101, hInstance_, NULL);

    CreateWindow(L"BUTTON", L"确定",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        110, 82, 70, 28, hDlg, (HMENU)IDOK, hInstance_, NULL);

    CreateWindow(L"BUTTON", L"取消",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 82, 70, 28, hDlg, (HMENU)IDCANCEL, hInstance_, NULL);

    ReleaseDC(hDlg, hdc);

    ShowWindow(hDlg, SW_SHOW);
    SetFocus(hEdit);

    MSG msg;
    BOOL ret;
    bool ok = false;
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) break;
        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!IsWindow(hDlg)) break;
    }

    if (IsWindow(hDlg)) {
        wchar_t buf[256] = {0};
        GetWindowText(GetDlgItem(hDlg, 101), buf, 256);
        result = buf;
        DestroyWindow(hDlg);
        ok = true;
    }

    return ok;
}

bool CalendarWindow::ShowAddCountdownDialog(std::wstring& name, std::wstring& date) {
    std::wstring nameResult = name;
    std::wstring dateResult = date;

    HWND hDlg = CreateWindow(L"#32770", L"添加倒计时",
        WS_DLGFRAME | WS_SYSMENU | DS_MODALFRAME,
        0, 0, 300, 180, hwnd_, NULL, hInstance_, NULL);

    RECT rc;
    GetWindowRect(hwnd_, &rc);
    int dlgW = 300, dlgH = 180;
    int x = rc.left + (WINDOW_WIDTH - dlgW) / 2;
    int y = rc.top + 80;
    SetWindowPos(hDlg, HWND_TOPMOST, x, y, dlgW, dlgH, SWP_SHOWWINDOW);

    HDC hdc = GetDC(hDlg);
    RECT r1 = {20, 12, 260, 32};
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0,0,0));
    DrawText(hdc, L"事件名称：", -1, &r1, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT r2 = {20, 62, 260, 82};
    DrawText(hdc, L"目标日期：", -1, &r2, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    ReleaseDC(hDlg, hdc);

    HWND hName = CreateWindow(L"EDIT", nameResult.c_str(),
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        20, 32, 260, 28, hDlg, (HMENU)101, hInstance_, NULL);

    HWND hDate = CreateWindow(L"EDIT", dateResult.c_str(),
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        20, 82, 260, 28, hDlg, (HMENU)102, hInstance_, NULL);

    CreateWindow(L"BUTTON", L"确定",
        WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        110, 122, 70, 28, hDlg, (HMENU)IDOK, hInstance_, NULL);

    CreateWindow(L"BUTTON", L"取消",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        200, 122, 70, 28, hDlg, (HMENU)IDCANCEL, hInstance_, NULL);

    SetFocus(hName);

    MSG msg;
    BOOL ret;
    bool ok = false;
    while ((ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) break;
        if (msg.hwnd == hDlg || IsChild(hDlg, msg.hwnd)) {
            if (msg.message == WM_COMMAND) {
                if (LOWORD(msg.wParam) == IDOK) {
                    wchar_t nbuf[128] = {0}, dbuf[64] = {0};
                    GetWindowText(hName, nbuf, 128);
                    GetWindowText(hDate, dbuf, 64);
                    name = nbuf;
                    date = dbuf;
                    ok = true;
                    break;
                }
                if (LOWORD(msg.wParam) == IDCANCEL) {
                    ok = false;
                    break;
                }
            }
            if (msg.message == WM_CLOSE) {
                ok = false;
                break;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hDlg);
    return ok && !name.empty() && !date.empty();
}

LRESULT CALLBACK CalendarWindow::WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        g_window = (CalendarWindow*)cs->lpCreateParams;
        g_window->hwnd_ = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)g_window);
    }

    CalendarWindow* p = (CalendarWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (p) return p->WndProc(msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CalendarWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT:
            OnPaint();
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_LBUTTONDOWN:
            OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_LBUTTONUP:
            OnLButtonUp();
            return 0;

        case WM_MOUSEMOVE:
            OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_RBUTTONUP:
            OnRButtonUp(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_DESTROY:
            SaveData();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd_, msg, wParam, lParam);
}

bool CalendarWindow::Create(HINSTANCE hInstance) {
    hInstance_ = hInstance;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProcStatic;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DesktopCalendar2024";
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassW(&wc)) {
        return false;
    }

    int height = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        L"DesktopCalendar2024",
        L"桌面日历",
        WS_POPUP | WS_VISIBLE,
        200, 200,
        WINDOW_WIDTH, height,
        NULL, NULL, hInstance, this
    );

    if (!hwnd_) return false;

    SetLayeredWindowAttributes(hwnd_, 0, 230, LWA_ALPHA);

    LoadData();

    SYSTEMTIME st;
    GetLocalTime(&st);
    int month = st.wMonth;
    if (month >= 3 && month <= 5) themeIndex_ = 0;
    else if (month >= 6 && month <= 8) themeIndex_ = 1;
    else if (month >= 9 && month <= 11) themeIndex_ = 2;
    else themeIndex_ = 3;

    return true;
}

void CalendarWindow::Show(int nCmdShow) {
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
}

static void RoundRect(HDC hdc, int x1, int y1, int x2, int y2, int r) {
    int d = r * 2;
    BeginPath(hdc);
    MoveToEx(hdc, x1 + r, y1, NULL);
    LineTo(hdc, x2 - r, y1);
    Arc(hdc, x2 - d, y1, x2, y1 + d, x2, y1, x2, y1 + r);
    LineTo(hdc, x2, y2 - r);
    Arc(hdc, x2 - d, y2 - d, x2, y2, x2, y2, x2 - r, y2);
    LineTo(hdc, x1 + r, y2);
    Arc(hdc, x1, y2 - d, x1 + d, y2, x1, y2, x1, y2 - r);
    LineTo(hdc, x1, y1 + r);
    Arc(hdc, x1, y1, x1 + d, y1 + d, x1, y1 + r, x1 + r, y1);
    EndPath(hdc);
}

static void FillRound(HDC hdc, int x1, int y1, int x2, int y2, int r, COLORREF color) {
    HBRUSH br = CreateSolidBrush(color);
    HBRUSH old = (HBRUSH)SelectObject(hdc, br);
    RoundRect(hdc, x1, y1, x2, y2, r);
    FillPath(hdc);
    SelectObject(hdc, old);
    DeleteObject(br);
}

void CalendarWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd_, &ps);

    RECT rc;
    GetClientRect(hwnd_, &rc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    ThemeColors& theme = g_themes[themeIndex_];

    HBRUSH bgBrush = CreateSolidBrush(theme.bgPrimary);
    FillRect(memDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    HRGN hRgn = CreateRoundRectRgn(0, 0, rc.right, rc.bottom, 28, 28);
    HBRUSH rgnBrush = CreateSolidBrush(theme.bgPrimary);
    FillRgn(memDC, hRgn, rgnBrush);
    DeleteObject(rgnBrush);
    DeleteObject(hRgn);

    PaintHeader(memDC);

    if (!isCollapsed_) {
        PaintCalendar(memDC);
        PaintTasks(memDC);
        PaintCountdowns(memDC);
    }

    PaintFooter(memDC);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hwnd_, &ps);
}

void CalendarWindow::PaintHeader(HDC hdc) {
    ThemeColors& theme = g_themes[themeIndex_];

    RECT titleRect = {40, 10, WINDOW_WIDTH - 40, 40};
    HFONT titleFont = CreateFont(-16, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.textPrimary);
    HFONT oldFont = (HFONT)SelectObject(hdc, titleFont);
    DrawText(hdc, L"✨ 每日任务", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);

    RECT btnRect = {10, 10, 34, 34};
    HFONT btnFont = CreateFont(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    oldFont = (HFONT)SelectObject(hdc, btnFont);
    SetTextColor(hdc, theme.accentDark);
    DrawText(hdc, isCollapsed_ ? L"≡" : L"≡", -1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(btnFont);
}

void CalendarWindow::PaintCalendar(HDC hdc) {
    ThemeColors& theme = g_themes[themeIndex_];
    SYSTEMTIME st;
    GetLocalTime(&st);

    int top = 50;

    std::wstring dayStr = std::to_wstring(st.wDay);
    RECT dayRect = {30, top, 110, top + 60};
    HFONT dayFont = CreateFont(-56, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.textPrimary);
    HFONT oldFont = (HFONT)SelectObject(hdc, dayFont);
    DrawText(hdc, dayStr.c_str(), -1, &dayRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(dayFont);

    std::wstring weekday = GetWeekdayName();
    std::wstring monthStr = std::to_wstring(st.wYear) + L"年" + std::to_wstring(st.wMonth) + L"月";

    RECT weekRect = {115, top + 5, 220, top + 30};
    HFONT weekFont = CreateFont(-14, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    oldFont = (HFONT)SelectObject(hdc, weekFont);
    SetTextColor(hdc, theme.textSecondary);
    DrawText(hdc, weekday.c_str(), -1, &weekRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(weekFont);

    RECT monthRect = {115, top + 28, 220, top + 50};
    HFONT monthFont = CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    oldFont = (HFONT)SelectObject(hdc, monthFont);
    SetTextColor(hdc, theme.accent);
    DrawText(hdc, monthStr.c_str(), -1, &monthRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(monthFont);
}

void CalendarWindow::PaintTasks(HDC hdc) {
    ThemeColors& theme = g_themes[themeIndex_];
    int y = 180;

    RECT headerRect = {20, y, WINDOW_WIDTH - 20, y + 24};
    HFONT sectionFont = CreateFont(-13, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.textPrimary);
    HFONT oldFont = (HFONT)SelectObject(hdc, sectionFont);
    DrawText(hdc, L"📝 今日任务", -1, &headerRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(sectionFont);

    RECT addRect = {WINDOW_WIDTH - 44, y, WINDOW_WIDTH - 20, y + 24};
    HFONT addFont = CreateFont(-20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    oldFont = (HFONT)SelectObject(hdc, addFont);
    SetTextColor(hdc, theme.accentDark);
    DrawText(hdc, L"+", -1, &addRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(addFont);

    y += 30;

    if (tasks_.empty()) {
        RECT emptyRect = {20, y, WINDOW_WIDTH - 20, y + 40};
        HFONT emptyFont = CreateFont(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
        oldFont = (HFONT)SelectObject(hdc, emptyFont);
        SetTextColor(hdc, RGB(180, 180, 180));
        DrawText(hdc, L"还没有任务，点击右上角 + 添加", -1, &emptyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        DeleteObject(emptyFont);
    } else {
        HFONT taskFont = CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");

        for (size_t i = 0; i < tasks_.size() && i < 4; i++) {
            const Task& t = tasks_[i];
            int itemY = y + (int)i * 36;

            FillRound(hdc, 20, itemY, WINDOW_WIDTH - 20, itemY + 32, 8,
                t.completed ? RGB(235, 235, 235) : RGB(255, 255, 255));

            if (t.completed) {
                HBRUSH br = CreateSolidBrush(theme.accent);
                RECT cr = {28, itemY + 6, 48, itemY + 26};
                FillRect(hdc, &cr, br);
                DeleteObject(br);
                oldFont = (HFONT)SelectObject(hdc, taskFont);
                SetTextColor(hdc, RGB(255,255,255));
                RECT tr = {28, itemY + 6, 48, itemY + 26};
                DrawText(hdc, L"✓", -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(hdc, oldFont);
            } else {
                HBRUSH br = CreateSolidBrush(RGB(255,255,255));
                HPEN pen = CreatePen(PS_SOLID, 2, theme.accent);
                HBRUSH oldBr = (HBRUSH)SelectObject(hdc, br);
                HPEN oldPen = (HPEN)SelectObject(hdc, pen);
                Rectangle(hdc, 30, itemY + 8, 46, itemY + 24);
                SelectObject(hdc, oldBr);
                SelectObject(hdc, oldPen);
                DeleteObject(br);
                DeleteObject(pen);
            }

            oldFont = (HFONT)SelectObject(hdc, taskFont);
            SetTextColor(hdc, t.completed ? RGB(170, 170, 170) : theme.textPrimary);
            RECT textRect = {58, itemY + 4, WINDOW_WIDTH - 60, itemY + 28};
            DrawText(hdc, t.text.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            SelectObject(hdc, oldFont);
        }
        DeleteObject(taskFont);
    }
}

void CalendarWindow::PaintCountdowns(HDC hdc) {
    ThemeColors& theme = g_themes[themeIndex_];

    int y = 350;
    if (tasks_.empty()) y = 290;

    RECT headerRect = {20, y, WINDOW_WIDTH - 20, y + 24};
    HFONT sectionFont = CreateFont(-13, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.textPrimary);
    HFONT oldFont = (HFONT)SelectObject(hdc, sectionFont);
    DrawText(hdc, L"⏰ 倒计时", -1, &headerRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(sectionFont);

    RECT addRect = {WINDOW_WIDTH - 44, y, WINDOW_WIDTH - 20, y + 24};
    HFONT addFont = CreateFont(-20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
    oldFont = (HFONT)SelectObject(hdc, addFont);
    SetTextColor(hdc, theme.accentDark);
    DrawText(hdc, L"+", -1, &addRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(addFont);

    y += 30;

    if (countdowns_.empty()) {
        RECT emptyRect = {20, y, WINDOW_WIDTH - 20, y + 40};
        HFONT emptyFont = CreateFont(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
        oldFont = (HFONT)SelectObject(hdc, emptyFont);
        SetTextColor(hdc, RGB(180, 180, 180));
        DrawText(hdc, L"还没有倒计时，点击右上角 + 添加", -1, &emptyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        DeleteObject(emptyFont);
    } else {
        HFONT nameFont = CreateFont(-13, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
        HFONT dateFont = CreateFont(-10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
        HFONT numFont = CreateFont(-22, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
        HFONT unitFont = CreateFont(-9, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");

        for (size_t i = 0; i < countdowns_.size() && i < 3; i++) {
            const Countdown& c = countdowns_[i];
            int itemY = y + (int)i * 44;

            FillRound(hdc, 20, itemY, WINDOW_WIDTH - 20, itemY + 40, 10, theme.bgSecondary);

            RECT iconRect = {28, itemY + 6, 60, itemY + 34};
            oldFont = (HFONT)SelectObject(hdc, nameFont);
            SetTextColor(hdc, theme.accentDark);
            DrawText(hdc, L"⏰", -1, &iconRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT nameRect = {68, itemY + 4, 200, itemY + 20};
            SetTextColor(hdc, theme.textPrimary);
            DrawText(hdc, c.name.c_str(), -1, &nameRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            size_t dot = c.targetDate.find(L'-');
            size_t dot2 = c.targetDate.find(L'-', dot + 1);
            std::wstring month = c.targetDate.substr(dot + 1, dot2 - dot - 1);
            std::wstring day = c.targetDate.substr(dot2 + 1);
            std::wstring dateStr = month + L"月" + day + L"日";

            RECT dateRect = {68, itemY + 20, 200, itemY + 34};
            oldFont = (HFONT)SelectObject(hdc, dateFont);
            SetTextColor(hdc, RGB(150, 150, 150));
            DrawText(hdc, dateStr.c_str(), -1, &dateRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            int days = CalculateDaysLeft(c.targetDate);
            std::wstring daysStr = std::to_wstring(days);

            RECT numRect = {WINDOW_WIDTH - 80, itemY + 2, WINDOW_WIDTH - 40, itemY + 28};
            oldFont = (HFONT)SelectObject(hdc, numFont);
            SetTextColor(hdc, theme.accentDark);
            DrawText(hdc, daysStr.c_str(), -1, &numRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT unitRect = {WINDOW_WIDTH - 80, itemY + 26, WINDOW_WIDTH - 40, itemY + 38};
            oldFont = (HFONT)SelectObject(hdc, unitFont);
            SetTextColor(hdc, theme.accent);
            DrawText(hdc, L"天", -1, &unitRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        SelectObject(hdc, oldFont);
        DeleteObject(nameFont);
        DeleteObject(dateFont);
        DeleteObject(numFont);
        DeleteObject(unitFont);
    }
}

void CalendarWindow::PaintFooter(HDC hdc) {
    ThemeColors& theme = g_themes[themeIndex_];
    int bottom = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;

    RECT footerRect = {0, bottom - 36, WINDOW_WIDTH, bottom};
    HFONT footerFont = CreateFont(-10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");

    std::wstring text = L"━━━ " + theme.seasonText + L" ━━━";
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.accent);
    HFONT oldFont = (HFONT)SelectObject(hdc, footerFont);
    DrawText(hdc, text.c_str(), -1, &footerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(footerFont);
}

void CalendarWindow::OnLButtonDown(int x, int y) {
    if (x >= 10 && x <= 34 && y >= 10 && y <= 34) {
        ToggleCollapse();
        return;
    }

    if (y < 45) {
        isDragging_ = true;
        POINT pt;
        GetCursorPos(&pt);
        RECT rc;
        GetWindowRect(hwnd_, &rc);
        dragOffset_.x = pt.x - rc.left;
        dragOffset_.y = pt.y - rc.top;
        SetCapture(hwnd_);
        return;
    }

    if (!isCollapsed_) {
        if (x >= WINDOW_WIDTH - 44 && x <= WINDOW_WIDTH - 20) {
            if (y >= 180 && y <= 204) {
                std::wstring text;
                if (ShowInputDialog(L"添加任务", L"请输入任务内容：", text) && !text.empty()) {
                    AddTask(text);
                }
                return;
            }

            int cy = tasks_.empty() ? 290 : 350;
            if (y >= cy && y <= cy + 24) {
                std::wstring name = L"中考";
                std::wstring date = L"2025-06-20";
                if (ShowAddCountdownDialog(name, date)) {
                    AddCountdown(name, date);
                }
                return;
            }
        }

        int taskY = 210;
        for (size_t i = 0; i < tasks_.size() && i < 4; i++) {
            int itemY = taskY + (int)i * 36;
            if (y >= itemY && y <= itemY + 32) {
                if (x >= 28 && x <= 48) {
                    ToggleTask(tasks_[i].id);
                    return;
                }
            }
        }
    }
}

void CalendarWindow::OnLButtonUp() {
    if (isDragging_) {
        isDragging_ = false;
        ReleaseCapture();
    }
}

void CalendarWindow::OnMouseMove(int x, int y) {
    if (isDragging_) {
        POINT pt;
        GetCursorPos(&pt);
        SetWindowPos(hwnd_, NULL,
            pt.x - dragOffset_.x, pt.y - dragOffset_.y,
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void CalendarWindow::OnRButtonUp(int x, int y) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, 1, isCollapsed_ ? L"展开" : L"折叠");
    AppendMenu(hMenu, MF_STRING, 2, L"切换主题");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, 3, L"退出");

    int cmd = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd_, NULL);

    if (cmd == 1) ToggleCollapse();
    else if (cmd == 2) NextTheme();
    else if (cmd == 3) DestroyWindow(hwnd_);

    DestroyMenu(hMenu);
}

void CalendarWindow::ToggleCollapse() {
    isCollapsed_ = !isCollapsed_;
    int height = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;
    SetWindowPos(hwnd_, NULL, 0, 0, WINDOW_WIDTH, height, SWP_NOMOVE | SWP_NOZORDER);
    InvalidateRect(hwnd_, NULL, TRUE);
    SaveData();
}

void CalendarWindow::NextTheme() {
    themeIndex_ = (themeIndex_ + 1) % 4;
    InvalidateRect(hwnd_, NULL, TRUE);
    SaveData();
}

std::wstring CalendarWindow::GetWeekdayName() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    const wchar_t* days[] = {L"星期日", L"星期一", L"星期二", L"星期三", L"星期四", L"星期五", L"星期六"};
    return days[st.wDayOfWeek];
}

int CalendarWindow::CalculateDaysLeft(const std::wstring& targetDate) {
    if (targetDate.size() < 10) return 0;

    int year = _wtoi(targetDate.substr(0, 4).c_str());
    int month = _wtoi(targetDate.substr(5, 2).c_str());
    int day = _wtoi(targetDate.substr(8, 2).c_str());

    SYSTEMTIME st;
    GetLocalTime(&st);

    FILETIME ftNow, ftTarget;
    SystemTimeToFileTime(&st, &ftNow);

    SYSTEMTIME tst = {0};
    tst.wYear = (WORD)year;
    tst.wMonth = (WORD)month;
    tst.wDay = (WORD)day;
    SystemTimeToFileTime(&tst, &ftTarget);

    ULONGLONG now = ((ULONGLONG)ftNow.dwHighDateTime << 32) | ftNow.dwLowDateTime;
    ULONGLONG target = ((ULONGLONG)ftTarget.dwHighDateTime << 32) | ftTarget.dwLowDateTime;

    LONGLONG diff = (LONGLONG)(target - now);
    int days = (int)(diff / (10000000LL * 60 * 60 * 24));
    return days > 0 ? days : 0;
}

void CalendarWindow::AddTask(const std::wstring& text) {
    if (text.empty()) return;
    Task t;
    t.id = (int)GetTickCount64();
    t.text = text;
    t.completed = false;
    tasks_.push_back(t);
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void CalendarWindow::ToggleTask(int id) {
    for (auto& t : tasks_) {
        if (t.id == id) {
            t.completed = !t.completed;
            break;
        }
    }
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void CalendarWindow::AddCountdown(const std::wstring& name, const std::wstring& targetDate) {
    if (name.empty() || targetDate.empty()) return;
    Countdown c;
    c.id = (int)GetTickCount64() + rand();
    c.name = name;
    c.targetDate = targetDate;
    countdowns_.push_back(c);
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

std::wstring CalendarWindow::GetDataFilePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hInstance_, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        dir = dir.substr(0, pos + 1);
    }
    return dir + L"data.txt";
}

static std::string WtoU8(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, NULL, 0, NULL, NULL);
    std::string r(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &r[0], len, NULL, NULL);
    return r;
}

static std::wstring U8toW(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    std::wstring r(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &r[0], len);
    return r;
}

void CalendarWindow::LoadData() {
    std::wstring path = GetDataFilePath();
    std::ifstream file(WtoU8(path));
    if (!file.is_open()) return;

    std::string line, section;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (line[0] == '[') {
            section = line.substr(1, line.size() - 2);
            continue;
        }

        if (section == "Settings") {
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            if (key == "theme") {
                themeIndex_ = atoi(val.c_str()) % 4;
            } else if (key == "collapsed") {
                isCollapsed_ = (val == "1");
            }
        } else if (section == "Tasks") {
            size_t p1 = line.find('|');
            if (p1 == std::string::npos) continue;
            size_t p2 = line.find('|', p1 + 1);
            if (p2 == std::string::npos) continue;
            Task t;
            t.id = atoi(line.substr(0, p1).c_str());
            t.completed = (line.substr(p1 + 1, p2 - p1 - 1) == "1");
            t.text = U8toW(line.substr(p2 + 1));
            tasks_.push_back(t);
        } else if (section == "Countdowns") {
            size_t p1 = line.find('|');
            if (p1 == std::string::npos) continue;
            size_t p2 = line.find('|', p1 + 1);
            if (p2 == std::string::npos) continue;
            Countdown c;
            c.id = atoi(line.substr(0, p1).c_str());
            c.name = U8toW(line.substr(p1 + 1, p2 - p1 - 1));
            c.targetDate = U8toW(line.substr(p2 + 1));
            countdowns_.push_back(c);
        }
    }
    file.close();
}

void CalendarWindow::SaveData() {
    std::wstring path = GetDataFilePath();
    std::ofstream file(WtoU8(path));
    if (!file.is_open()) return;

    file << "[Settings]\n";
    file << "theme=" << themeIndex_ << "\n";
    file << "collapsed=" << (isCollapsed_ ? "1" : "0") << "\n";
    file << "\n";

    file << "[Tasks]\n";
    for (const auto& t : tasks_) {
        file << t.id << "|" << (t.completed ? "1" : "0") << "|" << WtoU8(t.text) << "\n";
    }
    file << "\n";

    file << "[Countdowns]\n";
    for (const auto& c : countdowns_) {
        file << c.id << "|" << WtoU8(c.name) << "|" << WtoU8(c.targetDate) << "\n";
    }
    file << "\n";

    file.close();
}
