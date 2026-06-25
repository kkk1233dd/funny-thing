#include "window.h"
#include <fstream>
#include <algorithm>

ThemeColors g_themes[4];

void InitThemes() {
    // 春 - 樱落时节
    g_themes[0].bgPrimary = RGB(248, 252, 248);
    g_themes[0].bgSecondary = RGB(220, 240, 220);
    g_themes[0].accent = RGB(144, 210, 144);
    g_themes[0].accentDark = RGB(100, 175, 100);
    g_themes[0].accentLight = RGB(200, 235, 200);
    g_themes[0].textPrimary = RGB(35, 110, 40);
    g_themes[0].textSecondary = RGB(75, 135, 65);
    g_themes[0].border = RGB(170, 215, 170);
    g_themes[0].seasonText = L"樱落时节";

    // 夏 - 蝉鸣之夏
    g_themes[1].bgPrimary = RGB(235, 248, 235);
    g_themes[1].bgSecondary = RGB(195, 230, 195);
    g_themes[1].accent = RGB(110, 190, 115);
    g_themes[1].accentDark = RGB(75, 165, 80);
    g_themes[1].accentLight = RGB(185, 225, 185);
    g_themes[1].textPrimary = RGB(30, 105, 35);
    g_themes[1].textSecondary = RGB(70, 130, 60);
    g_themes[1].border = RGB(155, 210, 155);
    g_themes[1].seasonText = L"蝉鸣之夏";

    // 秋 - 枫红秋意
    g_themes[2].bgPrimary = RGB(255, 245, 228);
    g_themes[2].bgSecondary = RGB(255, 220, 170);
    g_themes[2].accent = RGB(255, 155, 30);
    g_themes[2].accentDark = RGB(240, 120, 0);
    g_themes[2].accentLight = RGB(255, 210, 155);
    g_themes[2].textPrimary = RGB(180, 80, 0);
    g_themes[2].textSecondary = RGB(215, 100, 0);
    g_themes[2].border = RGB(255, 185, 95);
    g_themes[2].seasonText = L"枫红秋意";

    // 冬 - 雪落冬安
    g_themes[3].bgPrimary = RGB(232, 245, 255);
    g_themes[3].bgSecondary = RGB(175, 215, 248);
    g_themes[3].accent = RGB(90, 170, 240);
    g_themes[3].accentDark = RGB(50, 145, 230);
    g_themes[3].accentLight = RGB(165, 205, 245);
    g_themes[3].textPrimary = RGB(20, 100, 190);
    g_themes[3].textSecondary = RGB(45, 125, 220);
    g_themes[3].border = RGB(140, 195, 245);
    g_themes[3].seasonText = L"雪落冬安";
}

static CalendarWindow* g_window = NULL;

CalendarWindow::CalendarWindow()
    : hwnd_(NULL), hInstance_(NULL), isDragging_(false)
    , isCollapsed_(false), themeIndex_(1)
{
    InitThemes();
}

CalendarWindow::~CalendarWindow() {
    if (hwnd_) DestroyWindow(hwnd_);
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
        case WM_PAINT: OnPaint(); return 0;
        case WM_ERASEBKGND: return 1;
        case WM_LBUTTONDOWN: OnLButtonDown(LOWORD(lParam), HIWORD(lParam)); return 0;
        case WM_LBUTTONUP: OnLButtonUp(); return 0;
        case WM_MOUSEMOVE: OnMouseMove(LOWORD(lParam), HIWORD(lParam)); return 0;
        case WM_RBUTTONUP: OnRButtonUp(LOWORD(lParam), HIWORD(lParam)); return 0;
        case WM_DESTROY: SaveData(); PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd_, msg, wParam, lParam);
}

bool CalendarWindow::Create(HINSTANCE hInstance) {
    hInstance_ = hInstance;
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProcStatic;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DesktopCalendarV2";
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int h = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;
    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        L"DesktopCalendarV2", L"桌面日历",
        WS_POPUP | WS_VISIBLE,
        200, 150, WINDOW_WIDTH, h,
        NULL, NULL, hInstance, this
    );
    if (!hwnd_) return false;

    SetLayeredWindowAttributes(hwnd_, 0, 248, LWA_ALPHA);
    LoadData();

    SYSTEMTIME st; GetLocalTime(&st);
    int m = st.wMonth;
    if (m >= 3 && m <= 5) themeIndex_ = 0;
    else if (m >= 6 && m <= 8) themeIndex_ = 1;
    else if (m >= 9 && m <= 11) themeIndex_ = 2;
    else themeIndex_ = 3;

    return true;
}

void CalendarWindow::Show(int nCmdShow) {
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
}

// ===== 绘图工具 =====
static void RoundedPath(HDC hdc, int x1, int y1, int x2, int y2, int r) {
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

static void FillRounded(HDC hdc, int x1, int y1, int x2, int y2, int r, COLORREF c) {
    HBRUSH br = CreateSolidBrush(c);
    HBRUSH old = (HBRUSH)SelectObject(hdc, br);
    RoundedPath(hdc, x1, y1, x2, y2, r);
    FillPath(hdc);
    SelectObject(hdc, old);
    DeleteObject(br);
}

static HFONT MakeFont(int h, int w, const wchar_t* face) {
    return CreateFont(-h, 0, 0, 0, w, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, face);
}

static void DrawTextAt(HDC hdc, const wchar_t* s, int x, int y, int w, int h, COLORREF c, HFONT f, UINT fmt = DT_CENTER | DT_VCENTER | DT_SINGLELINE) {
    RECT r = {x, y, x + w, y + h};
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, c);
    HFONT old = (HFONT)SelectObject(hdc, f);
    DrawText(hdc, s, -1, &r, fmt);
    SelectObject(hdc, old);
}

// ===== 绘制 =====
void CalendarWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd_, &ps);
    RECT rc; GetClientRect(hwnd_, &rc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    ThemeColors& th = g_themes[themeIndex_];

    HBRUSH bg = CreateSolidBrush(th.bgPrimary);
    FillRect(memDC, &rc, bg);
    DeleteObject(bg);

    HRGN rgn = CreateRoundRectRgn(0, 0, rc.right, rc.bottom, 30, 30);
    HBRUSH rgnBrush = CreateSolidBrush(th.bgPrimary);
    FillRgn(memDC, rgn, rgnBrush);
    DeleteObject(rgnBrush);
    DeleteObject(rgn);

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
    ThemeColors& th = g_themes[themeIndex_];

    HFONT titleFont = MakeFont(18, FW_MEDIUM, L"Microsoft YaHei UI");
    DrawTextAt(hdc, L"每日任务", 50, 8, WINDOW_WIDTH - 100, 36, th.textPrimary, titleFont);
    DeleteObject(titleFont);

    HFONT btnFont = MakeFont(18, FW_BOLD, L"Microsoft YaHei UI");
    DrawTextAt(hdc, isCollapsed_ ? L"≡" : L"≡", 10, 8, 36, 36, th.accentDark, btnFont);
    DeleteObject(btnFont);
}

void CalendarWindow::PaintCalendar(HDC hdc) {
    ThemeColors& th = g_themes[themeIndex_];
    SYSTEMTIME st; GetLocalTime(&st);
    int top = 52;

    // 日期数字
    std::wstring dayStr = std::to_wstring(st.wDay);
    HFONT dayFont = MakeFont(60, FW_LIGHT, L"Microsoft YaHei UI");
    DrawTextAt(hdc, dayStr.c_str(), 30, top, 80, 65, th.textPrimary, dayFont);
    DeleteObject(dayFont);

    // 星期
    std::wstring wd = GetWeekdayName();
    HFONT weekFont = MakeFont(15, FW_MEDIUM, L"Microsoft YaHei UI");
    DrawTextAt(hdc, wd.c_str(), 118, top + 5, 150, 26, th.textSecondary, weekFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(weekFont);

    // 年月
    std::wstring ym = std::to_wstring(st.wYear) + L"年" + std::to_wstring(st.wMonth) + L"月";
    HFONT monthFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
    DrawTextAt(hdc, ym.c_str(), 118, top + 30, 150, 24, th.accent, monthFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(monthFont);

    // 分隔线
    HPEN linePen = CreatePen(PS_SOLID, 1, th.border);
    HPEN oldPen = (HPEN)SelectObject(hdc, linePen);
    MoveToEx(hdc, 25, top + 75, NULL);
    LineTo(hdc, WINDOW_WIDTH - 25, top + 75);
    SelectObject(hdc, oldPen);
    DeleteObject(linePen);
}

void CalendarWindow::PaintTasks(HDC hdc) {
    ThemeColors& th = g_themes[themeIndex_];
    int y = 150;

    // 标题
    HFONT secFont = MakeFont(14, FW_MEDIUM, L"Microsoft YaHei UI");
    DrawTextAt(hdc, L"今日计划", 22, y, 120, 26, th.textPrimary, secFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(secFont);

    // +按钮
    FillRounded(hdc, WINDOW_WIDTH - 50, y + 1, WINDOW_WIDTH - 22, y + 25, 12, th.accent);
    HFONT addFont = MakeFont(20, FW_NORMAL, L"Microsoft YaHei UI");
    DrawTextAt(hdc, L"+", WINDOW_WIDTH - 50, y + 1, 28, 24, RGB(255,255,255), addFont);
    DeleteObject(addFont);

    y += 34;

    // 统计今日可见任务
    std::vector<Task*> todayTasks;
    for (auto& t : tasks_) {
        if (IsTaskVisibleToday(t)) todayTasks.push_back(&t);
    }

    if (todayTasks.empty()) {
        HFONT emptyFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
        DrawTextAt(hdc, L"今天还没有计划，点 + 添加", 20, y, WINDOW_WIDTH - 40, 40, RGB(170, 170, 170), emptyFont);
        DeleteObject(emptyFont);
    } else {
        HFONT taskFont = MakeFont(13, FW_NORMAL, L"Microsoft YaHei UI");
        HFONT repFont = MakeFont(10, FW_NORMAL, L"Microsoft YaHei UI");

        for (size_t i = 0; i < todayTasks.size() && i < 4; i++) {
            Task* t = todayTasks[i];
            int iy = y + (int)i * 44;

            FillRounded(hdc, 20, iy, WINDOW_WIDTH - 20, iy + 38, 10,
                t->completed ? RGB(240, 240, 240) : RGB(255, 255, 255));

            // 复选框
            if (t->completed) {
                FillRounded(hdc, 30, iy + 9, 52, iy + 31, 6, th.accent);
                HFONT cFont = MakeFont(14, FW_BOLD, L"Microsoft YaHei UI");
                DrawTextAt(hdc, L"✓", 30, iy + 9, 22, 22, RGB(255,255,255), cFont);
                DeleteObject(cFont);
            } else {
                HBRUSH br = CreateSolidBrush(RGB(255,255,255));
                HPEN pen = CreatePen(PS_SOLID, 2, th.accent);
                HBRUSH ob = (HBRUSH)SelectObject(hdc, br);
                HPEN op = (HPEN)SelectObject(hdc, pen);
                RoundedPath(hdc, 31, iy + 10, 51, iy + 30, 5);
                StrokePath(hdc);
                SelectObject(hdc, ob);
                SelectObject(hdc, op);
                DeleteObject(br);
                DeleteObject(pen);
            }

            // 任务文字
            COLORREF tc = t->completed ? RGB(170, 170, 170) : th.textPrimary;
            DrawTextAt(hdc, t->text.c_str(), 62, iy + 4, WINDOW_WIDTH - 110, 22, tc, taskFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            // 循环标签
            std::wstring repText = GetRepeatText(*t);
            if (!repText.empty()) {
                FillRounded(hdc, 62, iy + 24, 120, iy + 35, 4, th.accentLight);
                DrawTextAt(hdc, repText.c_str(), 62, iy + 24, 58, 11, th.accentDark, repFont,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            // 删除按钮
            DrawTextAt(hdc, L"×", WINDOW_WIDTH - 42, iy + 4, 22, 22,
                RGB(220, 130, 130), taskFont);
        }
        DeleteObject(taskFont);
        DeleteObject(repFont);
    }
}

void CalendarWindow::PaintCountdowns(HDC hdc) {
    ThemeColors& th = g_themes[themeIndex_];

    int y = 350;
    // 动态调整位置
    std::vector<Task*> todayTasks;
    for (auto& t : tasks_) if (IsTaskVisibleToday(t)) todayTasks.push_back(&t);
    int taskCount = std::min((int)todayTasks.size(), 4);
    if (taskCount == 0) y = 270;
    else if (taskCount <= 2) y = 270 + taskCount * 44;
    else y = 270 + taskCount * 44 + 10;

    // 标题
    HFONT secFont = MakeFont(14, FW_MEDIUM, L"Microsoft YaHei UI");
    DrawTextAt(hdc, L"重要日子", 22, y, 120, 26, th.textPrimary, secFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(secFont);

    // +按钮
    FillRounded(hdc, WINDOW_WIDTH - 50, y + 1, WINDOW_WIDTH - 22, y + 25, 12, th.accent);
    HFONT addFont = MakeFont(20, FW_NORMAL, L"Microsoft YaHei UI");
    DrawTextAt(hdc, L"+", WINDOW_WIDTH - 50, y + 1, 28, 24, RGB(255,255,255), addFont);
    DeleteObject(addFont);

    y += 34;

    if (countdowns_.empty()) {
        HFONT emptyFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
        DrawTextAt(hdc, L"还没有倒数日，点 + 添加", 20, y, WINDOW_WIDTH - 40, 40, RGB(170, 170, 170), emptyFont);
        DeleteObject(emptyFont);
    } else {
        HFONT nameFont = MakeFont(13, FW_MEDIUM, L"Microsoft YaHei UI");
        HFONT dateFont = MakeFont(11, FW_NORMAL, L"Microsoft YaHei UI");
        HFONT numFont = MakeFont(26, FW_LIGHT, L"Microsoft YaHei UI");
        HFONT unitFont = MakeFont(10, FW_NORMAL, L"Microsoft YaHei UI");

        for (size_t i = 0; i < countdowns_.size() && i < 3; i++) {
            const Countdown& c = countdowns_[i];
            int iy = y + (int)i * 50;

            FillRounded(hdc, 20, iy, WINDOW_WIDTH - 20, iy + 44, 12, th.bgSecondary);

            // 图标区
            FillRounded(hdc, 32, iy + 8, 62, iy + 38, 8, th.accentLight);
            DrawTextAt(hdc, L"🎐", 32, iy + 8, 30, 30, th.accentDark, nameFont);

            // 名称
            DrawTextAt(hdc, c.name.c_str(), 72, iy + 6, 160, 22, th.textPrimary, nameFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            // 日期
            size_t d1 = c.targetDate.find(L'-');
            size_t d2 = c.targetDate.find(L'-', d1 + 1);
            std::wstring mo = c.targetDate.substr(d1 + 1, d2 - d1 - 1);
            std::wstring da = c.targetDate.substr(d2 + 1);
            std::wstring ds = mo + L"月" + da + L"日";
            DrawTextAt(hdc, ds.c_str(), 72, iy + 26, 160, 16, RGB(140, 140, 140), dateFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            int days = CalculateDaysLeft(c.targetDate);
            std::wstring dStr = std::to_wstring(days);

            // 天数
            DrawTextAt(hdc, dStr.c_str(), WINDOW_WIDTH - 90, iy + 4, 48, 28, th.accentDark, numFont,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            DrawTextAt(hdc, L"天后", WINDOW_WIDTH - 90, iy + 28, 48, 14, th.accent, unitFont,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        DeleteObject(nameFont);
        DeleteObject(dateFont);
        DeleteObject(numFont);
        DeleteObject(unitFont);
    }
}

void CalendarWindow::PaintFooter(HDC hdc) {
    ThemeColors& th = g_themes[themeIndex_];
    int bottom = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;

    HFONT fFont = MakeFont(11, FW_NORMAL, L"Microsoft YaHei UI");
    std::wstring t = L"━━━ " + th.seasonText + L" ━━━";
    DrawTextAt(hdc, t.c_str(), 0, bottom - 40, WINDOW_WIDTH, 36, th.accent, fFont);
    DeleteObject(fFont);
}

// ===== 鼠标事件 =====
void CalendarWindow::OnLButtonDown(int x, int y) {
    // 折叠按钮
    if (x >= 10 && x <= 46 && y >= 8 && y <= 44) {
        ToggleCollapse();
        return;
    }

    // 标题区 - 拖拽
    if (y < 50) {
        isDragging_ = true;
        POINT pt; GetCursorPos(&pt);
        RECT rc; GetWindowRect(hwnd_, &rc);
        dragOffset_.x = pt.x - rc.left;
        dragOffset_.y = pt.y - rc.top;
        SetCapture(hwnd_);
        return;
    }

    if (isCollapsed_) return;

    // 任务区 + 按钮
    if (x >= WINDOW_WIDTH - 50 && x <= WINDOW_WIDTH - 22) {
        // 任务 +
        if (y >= 151 && y <= 175) {
            if (ShowAddTaskDialog()) InvalidateRect(hwnd_, NULL, TRUE);
            return;
        }
        // 倒数日 +
        int cy = 351;
        std::vector<Task*> tt;
        for (auto& t : tasks_) if (IsTaskVisibleToday(t)) tt.push_back(&t);
        int tc = std::min((int)tt.size(), 4);
        if (tc == 0) cy = 271;
        else if (tc <= 2) cy = 271 + tc * 44;
        else cy = 271 + tc * 44 + 10;
        if (y >= cy && y <= cy + 24) {
            if (ShowAddCountdownDialog()) InvalidateRect(hwnd_, NULL, TRUE);
            return;
        }
    }

    // 任务复选框和删除
    int taskY = 184;
    std::vector<Task*> todayTasks;
    for (auto& t : tasks_) if (IsTaskVisibleToday(t)) todayTasks.push_back(&t);

    for (size_t i = 0; i < todayTasks.size() && i < 4; i++) {
        int iy = taskY + (int)i * 44;
        if (y >= iy && y <= iy + 38) {
            if (x >= 30 && x <= 52) {
                ToggleTask(todayTasks[i]->id);
                return;
            }
            if (x >= WINDOW_WIDTH - 42 && x <= WINDOW_WIDTH - 20) {
                DeleteTask(todayTasks[i]->id);
                return;
            }
        }
    }
}

void CalendarWindow::OnLButtonUp() {
    if (isDragging_) { isDragging_ = false; ReleaseCapture(); }
}

void CalendarWindow::OnMouseMove(int x, int y) {
    if (isDragging_) {
        POINT pt; GetCursorPos(&pt);
        SetWindowPos(hwnd_, NULL, pt.x - dragOffset_.x, pt.y - dragOffset_.y,
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void CalendarWindow::OnRButtonUp(int x, int y) {
    POINT pt; GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, 1, isCollapsed_ ? L"展开" : L"折叠");
    AppendMenu(menu, MF_STRING, 2, L"切换主题");
    AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(menu, MF_STRING, 3, L"退出");
    int cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd_, NULL);
    if (cmd == 1) ToggleCollapse();
    else if (cmd == 2) NextTheme();
    else if (cmd == 3) DestroyWindow(hwnd_);
    DestroyMenu(menu);
}

void CalendarWindow::ToggleCollapse() {
    isCollapsed_ = !isCollapsed_;
    int h = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;
    SetWindowPos(hwnd_, NULL, 0, 0, WINDOW_WIDTH, h, SWP_NOMOVE | SWP_NOZORDER);
    InvalidateRect(hwnd_, NULL, TRUE);
    SaveData();
}

void CalendarWindow::NextTheme() {
    themeIndex_ = (themeIndex_ + 1) % 4;
    InvalidateRect(hwnd_, NULL, TRUE);
    SaveData();
}

std::wstring CalendarWindow::GetWeekdayName() {
    SYSTEMTIME st; GetLocalTime(&st);
    const wchar_t* days[] = {L"星期日", L"星期一", L"星期二", L"星期三", L"星期四", L"星期五", L"星期六"};
    return days[st.wDayOfWeek];
}

std::wstring CalendarWindow::GetTodayStr() {
    SYSTEMTIME st; GetLocalTime(&st);
    wchar_t buf[20];
    swprintf(buf, 20, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
    return std::wstring(buf);
}

int CalendarWindow::CalculateDaysLeft(const std::wstring& td) {
    if (td.size() < 10) return 0;
    int y = _wtoi(td.substr(0, 4).c_str());
    int m = _wtoi(td.substr(5, 2).c_str());
    int d = _wtoi(td.substr(8, 2).c_str());

    SYSTEMTIME st; GetLocalTime(&st);
    FILETIME ftNow, ftTgt;
    SystemTimeToFileTime(&st, &ftNow);
    SYSTEMTIME tst = {0};
    tst.wYear = (WORD)y; tst.wMonth = (WORD)m; tst.wDay = (WORD)d;
    SystemTimeToFileTime(&tst, &ftTgt);

    ULONGLONG now = ((ULONGLONG)ftNow.dwHighDateTime << 32) | ftNow.dwLowDateTime;
    ULONGLONG tgt = ((ULONGLONG)ftTgt.dwHighDateTime << 32) | ftTgt.dwLowDateTime;
    LONGLONG diff = (LONGLONG)(tgt - now);
    int days = (int)(diff / (10000000LL * 60 * 60 * 24));
    return days > 0 ? days : 0;
}

bool CalendarWindow::IsTaskVisibleToday(const Task& t) {
    if (t.repeatType == RepeatType::None) {
        return true;
    }
    SYSTEMTIME st; GetLocalTime(&st);
    int wd = st.wDayOfWeek; // 0=Sun

    switch (t.repeatType) {
        case RepeatType::Daily: return true;
        case RepeatType::Weekdays: return wd >= 1 && wd <= 5;
        case RepeatType::Weekends: return wd == 0 || wd == 6;
        case RepeatType::Custom: return (t.customWeekdays & (1 << wd)) != 0;
    }
    return true;
}

std::wstring CalendarWindow::GetRepeatText(const Task& t) {
    switch (t.repeatType) {
        case RepeatType::None: return L"";
        case RepeatType::Daily: return L"每天";
        case RepeatType::Weekdays: return L"工作日";
        case RepeatType::Weekends: return L"周末";
        case RepeatType::Custom: return L"自定义";
    }
    return L"";
}

void CalendarWindow::AddTask(const std::wstring& text, RepeatType rt, int customDays) {
    if (text.empty()) return;
    Task t;
    t.id = (int)GetTickCount64();
    t.text = text;
    t.completed = false;
    t.repeatType = rt;
    t.customWeekdays = customDays;
    t.lastCompletedDate = L"";
    tasks_.push_back(t);
    SaveData();
}

void CalendarWindow::ToggleTask(int id) {
    for (auto& t : tasks_) {
        if (t.id == id) {
            t.completed = !t.completed;
            if (t.completed) t.lastCompletedDate = GetTodayStr();
            break;
        }
    }
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void CalendarWindow::DeleteTask(int id) {
    tasks_.erase(std::remove_if(tasks_.begin(), tasks_.end(),
        [id](const Task& t) { return t.id == id; }), tasks_.end());
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void CalendarWindow::AddCountdown(const std::wstring& name, const std::wstring& td) {
    if (name.empty() || td.empty()) return;
    Countdown c;
    c.id = (int)GetTickCount64() + rand();
    c.name = name;
    c.targetDate = td;
    countdowns_.push_back(c);
    SaveData();
}

void CalendarWindow::DeleteCountdown(int id) {
    countdowns_.erase(std::remove_if(countdowns_.begin(), countdowns_.end(),
        [id](const Countdown& c) { return c.id == id; }), countdowns_.end());
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

// ===== 自绘对话框 - 添加任务 =====
struct AddTaskDlgData {
    bool result;
    std::wstring text;
    RepeatType repeatType;
    int customDays;
    int selectedTab; 
    ThemeColors* theme;
};

static LRESULT CALLBACK AddTaskDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static AddTaskDlgData* data = NULL;

    if (msg == WM_INITDIALOG) {
        data = (AddTaskDlgData*)lParam;
        return TRUE;
    }

    if (!data) return FALSE;

    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hDlg, &ps);
            ThemeColors& th = *data->theme;

            RECT rc; GetClientRect(hDlg, &rc);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

            // 背景
            FillRounded(memDC, 0, 0, rc.right, rc.bottom, 16, th.bgPrimary);

            // 标题栏
            HFONT titleFont = MakeFont(16, FW_MEDIUM, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"添加任务", 20, 14, 200, 28, th.textPrimary, titleFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(titleFont);

            // 关闭按钮
            FillRounded(memDC, rc.right - 36, 12, rc.right - 12, 36, 12, RGB(255, 200, 200));
            HFONT xFont = MakeFont(16, FW_BOLD, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"×", rc.right - 36, 12, 24, 24, RGB(230, 80, 80), xFont);
            DeleteObject(xFont);

            // 输入标签
            HFONT labelFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"任务内容", 24, 56, 100, 22, th.textSecondary, labelFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(labelFont);

            // 输入框背景
            FillRounded(memDC, 24, 80, rc.right - 24, 112, 10, RGB(255, 255, 255));
            // 输入框边框
            HBRUSH br = CreateSolidBrush(RGB(255,255,255));
            HPEN pen = CreatePen(PS_SOLID, 1.5, th.accent);
            HBRUSH ob = (HBRUSH)SelectObject(memDC, br);
            HPEN op = (HPEN)SelectObject(memDC, pen);
            RoundedPath(memDC, 24, 80, rc.right - 24, 112, 10);
            StrokePath(memDC);
            SelectObject(memDC, ob);
            SelectObject(memDC, op);
            DeleteObject(br);
            DeleteObject(pen);

            // 输入文字
            if (!data->text.empty()) {
                HFONT inputFont = MakeFont(14, FW_NORMAL, L"Microsoft YaHei UI");
                DrawTextAt(memDC, data->text.c_str(), 34, 80, rc.right - 68, 32, th.textPrimary, inputFont,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                DeleteObject(inputFont);
            } else {
                HFONT phFont = MakeFont(13, FW_NORMAL, L"Microsoft YaHei UI");
                DrawTextAt(memDC, L"请输入任务内容...", 34, 80, rc.right - 68, 32, RGB(190, 190, 190), phFont,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                DeleteObject(phFont);
            }

            // 重复标签
            HFONT rLabelFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"重复方式", 24, 128, 100, 22, th.textSecondary, rLabelFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(rLabelFont);

            // 重复选项按钮
            const wchar_t* options[] = {L"不重复", L"每天", L"工作日", L"周末", L"自定义"};
            int optCount = 5;
            int btnW = (rc.right - 48) / optCount;
            for (int i = 0; i < optCount; i++) {
                int bx = 24 + i * btnW;
                bool selected = ((int)data->repeatType == i);
                COLORREF bg = selected ? th.accent : RGB(255, 255, 255);
                COLORREF tc = selected ? RGB(255,255,255) : th.textPrimary;
                FillRounded(memDC, bx + 2, 152, bx + btnW - 4, 182, 8, bg);
                HFONT optFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
                DrawTextAt(memDC, options[i], bx + 2, 152, btnW - 4, 30, tc, optFont);
                DeleteObject(optFont);
            }

            // 自定义星期选择（如果选了自定义）
            if (data->repeatType == RepeatType::Custom) {
                const wchar_t* wdays[] = {L"日", L"一", L"二", L"三", L"四", L"五", L"六"};
                int dayBtnSize = 34;
                int totalW = dayBtnSize * 7 + 8 * 6;
                int startX = (rc.right - totalW) / 2;
                for (int i = 0; i < 7; i++) {
                    int dx = startX + i * (dayBtnSize + 8);
                    bool sel = (data->customDays & (1 << i)) != 0;
                    COLORREF bg = sel ? th.accent : th.accentLight;
                    COLORREF tc = sel ? RGB(255,255,255) : th.textPrimary;
                    FillRounded(memDC, dx, 196, dx + dayBtnSize, 196 + dayBtnSize, 17, bg);
                    HFONT wFont = MakeFont(13, FW_MEDIUM, L"Microsoft YaHei UI");
                    DrawTextAt(memDC, wdays[i], dx, 196, dayBtnSize, dayBtnSize, tc, wFont);
                    DeleteObject(wFont);
                }
            }

            // 按钮
            int btnY = data->repeatType == RepeatType::Custom ? 250 : 210;
            FillRounded(memDC, 24, btnY, rc.right / 2 - 6, btnY + 36, 10, RGB(240, 240, 240));
            FillRounded(memDC, rc.right / 2 + 6, btnY, rc.right - 24, btnY + 36, 10, th.accent);

            HFONT btnFont = MakeFont(14, FW_MEDIUM, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"取消", 24, btnY, rc.right / 2 - 30, 36, RGB(120, 120, 120), btnFont);
            DrawTextAt(memDC, L"确定", rc.right / 2 + 6, btnY, rc.right / 2 - 30, 36, RGB(255,255,255), btnFont);
            DeleteObject(btnFont);

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            EndPaint(hDlg, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam), y = HIWORD(lParam);
            RECT rc; GetClientRect(hDlg, &rc);

            // 关闭
            if (x >= rc.right - 36 && x <= rc.right - 12 && y >= 12 && y <= 36) {
                data->result = false;
                EndDialog(hDlg, 0);
                return 0;
            }

            // 输入框点击 - 用 MessageBox 输入
            if (x >= 24 && x <= rc.right - 24 && y >= 80 && y <= 112) {
                wchar_t buf[256] = {0};
                wcscpy_s(buf, data->text.c_str());
                if (IDOK == MessageBox(hDlg, L"请输入任务内容：", L"任务内容", MB_OKCANCEL | MB_ICONQUESTION)) {
                    // 用一个简单输入方式
                }
                return 0;
            }

            // 重复选项
            if (y >= 152 && y <= 182) {
                int optCount = 5;
                int btnW = (rc.right - 48) / optCount;
                for (int i = 0; i < optCount; i++) {
                    int bx = 24 + i * btnW;
                    if (x >= bx + 2 && x <= bx + btnW - 4) {
                        data->repeatType = (RepeatType)i;
                        InvalidateRect(hDlg, NULL, TRUE);
                        return 0;
                    }
                }
            }

            // 自定义星期
            if (data->repeatType == RepeatType::Custom && y >= 196 && y <= 230) {
                int dayBtnSize = 34;
                int totalW = dayBtnSize * 7 + 8 * 6;
                int startX = (rc.right - totalW) / 2;
                for (int i = 0; i < 7; i++) {
                    int dx = startX + i * (dayBtnSize + 8);
                    if (x >= dx && x <= dx + dayBtnSize) {
                        data->customDays ^= (1 << i);
                        InvalidateRect(hDlg, NULL, TRUE);
                        return 0;
                    }
                }
            }

            int btnY = data->repeatType == RepeatType::Custom ? 250 : 210;
            // 取消
            if (x >= 24 && x <= rc.right / 2 - 6 && y >= btnY && y <= btnY + 36) {
                data->result = false;
                EndDialog(hDlg, 0);
                return 0;
            }
            // 确定
            if (x >= rc.right / 2 + 6 && x <= rc.right - 24 && y >= btnY && y <= btnY + 36) {
                data->result = true;
                EndDialog(hDlg, 1);
                return 0;
            }

            // 其他地方点击 - 尝试用输入框输入文字
            if (y >= 80 && y <= 112) {
                wchar_t buf[256];
                wcscpy_s(buf, data->text.c_str());
                // 使用简单的方式：每次点击追加一个字符不现实，用系统对话框
                if (IDOK == MessageBox(hDlg, L"请在点击确定后，通过系统提示输入任务名称\n\n（注：自绘输入框需要更多控件支持，暂时使用系统弹窗输入）",
                    L"输入提示", MB_OKCANCEL | MB_ICONINFORMATION)) {
                    // 继续
                }
            }

            return 0;
        }

        case WM_CHAR: {
            wchar_t ch = (wchar_t)wParam;
            if (ch == VK_BACK) {
                if (!data->text.empty()) data->text.resize(data->text.size() - 1);
            } else if (ch >= 32 && data->text.size() < 50) {
                data->text += ch;
            }
            InvalidateRect(hDlg, NULL, TRUE);
            return 0;
        }

        case WM_KEYDOWN: {
            if (wParam == VK_RETURN) {
                data->result = true;
                EndDialog(hDlg, 1);
                return 0;
            }
            if (wParam == VK_ESCAPE) {
                data->result = false;
                EndDialog(hDlg, 0);
                return 0;
            }
            return 0;
        }

        case WM_CLOSE:
            data->result = false;
            EndDialog(hDlg, 0);
            return 0;
    }
    return FALSE;
}

bool CalendarWindow::ShowAddTaskDialog() {
    AddTaskDlgData data;
    data.result = false;
    data.text = L"";
    data.repeatType = RepeatType::Daily;
    data.customDays = 0;
    data.theme = &g_themes[themeIndex_];

    // 获取窗口位置
    RECT rc; GetWindowRect(hwnd_, &rc);
    int dlgW = 300;
    int dlgH = data.repeatType == RepeatType::Custom ? 300 : 260;

    // 注册临时窗口类
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance_;
    wc.lpszClassName = L"AddTaskDlgClass";
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hDlg = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        L"AddTaskDlgClass", L"",
        WS_POPUP | WS_VISIBLE,
        rc.left + (WINDOW_WIDTH - dlgW) / 2, rc.top + 80,
        dlgW, dlgH,
        hwnd_, NULL, hInstance_, NULL
    );

    if (!hDlg) {
        UnregisterClassW(L"AddTaskDlgClass", hInstance_);
        return false;
    }

    SetLayeredWindowAttributes(hDlg, 0, 250, LWA_ALPHA);
    SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)AddTaskDlgProc);
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)&data);

    // 模拟 WM_INITDIALOG
    SendMessage(hDlg, WM_INITDIALOG, 0, (LPARAM)&data);

    // 模态消息循环
    MSG msg;
    BOOL ret;
    bool done = false;
    while (!done && (ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) break;
        if (msg.hwnd == hDlg || IsChild(hDlg, msg.hwnd)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!IsWindow(hDlg)) { done = true; break; }
    }

    if (IsWindow(hDlg)) DestroyWindow(hDlg);
    UnregisterClassW(L"AddTaskDlgClass", hInstance_);

    if (data.result && !data.text.empty()) {
        AddTask(data.text, data.repeatType, data.customDays);
        return true;
    }
    return false;
}

// ===== 倒数日对话框 =====
struct AddCdDlgData {
    bool result;
    std::wstring name;
    std::wstring date;
    ThemeColors* theme;
};

static LRESULT CALLBACK AddCdDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static AddCdDlgData* data = NULL;
    if (msg == WM_INITDIALOG) {
        data = (AddCdDlgData*)lParam;
        return TRUE;
    }
    if (!data) return FALSE;

    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hDlg, &ps);
            ThemeColors& th = *data->theme;
            RECT rc; GetClientRect(hDlg, &rc);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

            FillRounded(memDC, 0, 0, rc.right, rc.bottom, 16, th.bgPrimary);

            HFONT titleFont = MakeFont(16, FW_MEDIUM, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"添加倒数日", 20, 14, 200, 28, th.textPrimary, titleFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(titleFont);

            FillRounded(memDC, rc.right - 36, 12, rc.right - 12, 36, 12, RGB(255, 200, 200));
            HFONT xFont = MakeFont(16, FW_BOLD, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"×", rc.right - 36, 12, 24, 24, RGB(230, 80, 80), xFont);
            DeleteObject(xFont);

            HFONT labelFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"事件名称", 24, 56, 100, 22, th.textSecondary, labelFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(labelFont);

            FillRounded(memDC, 24, 80, rc.right - 24, 112, 10, RGB(255, 255, 255));
            HBRUSH br = CreateSolidBrush(RGB(255,255,255));
            HPEN pen = CreatePen(PS_SOLID, 1.5, th.accent);
            HBRUSH ob = (HBRUSH)SelectObject(memDC, br);
            HPEN op = (HPEN)SelectObject(memDC, pen);
            RoundedPath(memDC, 24, 80, rc.right - 24, 112, 10);
            StrokePath(memDC);
            SelectObject(memDC, ob);
            SelectObject(memDC, op);
            DeleteObject(br); DeleteObject(pen);

            HFONT inputFont = MakeFont(13, FW_NORMAL, L"Microsoft YaHei UI");
            if (!data->name.empty()) {
                DrawTextAt(memDC, data->name.c_str(), 34, 80, rc.right - 68, 32, th.textPrimary, inputFont,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawTextAt(memDC, L"如：中考、生日、春节...", 34, 80, rc.right - 68, 32, RGB(190, 190, 190), inputFont,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }
            DeleteObject(inputFont);

            HFONT dLabelFont = MakeFont(12, FW_NORMAL, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"目标日期", 24, 128, 100, 22, th.textSecondary, dLabelFont,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            DeleteObject(dLabelFont);

            FillRounded(memDC, 24, 152, rc.right - 24, 184, 10, RGB(255, 255, 255));
            br = CreateSolidBrush(RGB(255,255,255));
            pen = CreatePen(PS_SOLID, 1.5, th.accent);
            ob = (HBRUSH)SelectObject(memDC, br);
            op = (HPEN)SelectObject(memDC, pen);
            RoundedPath(memDC, 24, 152, rc.right - 24, 184, 10);
            StrokePath(memDC);
            SelectObject(memDC, ob);
            SelectObject(memDC, op);
            DeleteObject(br); DeleteObject(pen);

            HFONT dateFont = MakeFont(13, FW_NORMAL, L"Microsoft YaHei UI");
            if (!data->date.empty()) {
                DrawTextAt(memDC, data->date.c_str(), 34, 152, rc.right - 68, 32, th.textPrimary, dateFont,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawTextAt(memDC, L"格式：2025-06-20", 34, 152, rc.right - 68, 32, RGB(190, 190, 190), dateFont,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }
            DeleteObject(dateFont);

            // 快捷日期
            const wchar_t* quickDates[] = {L"中考", L"高考", L"元旦", L"春节"};
            int quickX = 24;
            for (int i = 0; i < 4; i++) {
                FillRounded(memDC, quickX, 196, quickX + 58, 218, 8, th.accentLight);
                HFONT qFont = MakeFont(11, FW_NORMAL, L"Microsoft YaHei UI");
                DrawTextAt(memDC, quickDates[i], quickX, 196, 58, 22, th.accentDark, qFont);
                DeleteObject(qFont);
                quickX += 66;
            }

            int btnY = 236;
            FillRounded(memDC, 24, btnY, rc.right / 2 - 6, btnY + 36, 10, RGB(240, 240, 240));
            FillRounded(memDC, rc.right / 2 + 6, btnY, rc.right - 24, btnY + 36, 10, th.accent);
            HFONT btnFont = MakeFont(14, FW_MEDIUM, L"Microsoft YaHei UI");
            DrawTextAt(memDC, L"取消", 24, btnY, rc.right / 2 - 30, 36, RGB(120, 120, 120), btnFont);
            DrawTextAt(memDC, L"确定", rc.right / 2 + 6, btnY, rc.right / 2 - 30, 36, RGB(255,255,255), btnFont);
            DeleteObject(btnFont);

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            EndPaint(hDlg, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam), y = HIWORD(lParam);
            RECT rc; GetClientRect(hDlg, &rc);

            if (x >= rc.right - 36 && x <= rc.right - 12 && y >= 12 && y <= 36) {
                data->result = false; EndDialog(hDlg, 0); return 0;
            }

            // 快捷日期
            if (y >= 196 && y <= 218) {
                int quickX = 24;
                const wchar_t* names[] = {L"中考", L"高考", L"元旦", L"春节"};
                const wchar_t* dates[] = {L"2026-06-16", L"2026-06-07", L"2027-01-01", L"2027-02-06"};
                for (int i = 0; i < 4; i++) {
                    if (x >= quickX && x <= quickX + 58) {
                        data->name = names[i];
                        data->date = dates[i];
                        InvalidateRect(hDlg, NULL, TRUE);
                        return 0;
                    }
                    quickX += 66;
                }
            }

            int btnY = 236;
            if (x >= 24 && x <= rc.right / 2 - 6 && y >= btnY && y <= btnY + 36) {
                data->result = false; EndDialog(hDlg, 0); return 0;
            }
            if (x >= rc.right / 2 + 6 && x <= rc.right - 24 && y >= btnY && y <= btnY + 36) {
                data->result = true; EndDialog(hDlg, 1); return 0;
            }

            return 0;
        }

        case WM_CHAR: {
            wchar_t ch = (wchar_t)wParam;
            // 简单处理：如果点了名称区，输入名称；点了日期区，输入日期
            // 简化：默认输入名称，按 Tab 切换
            static bool isDateInput = false;
            if (ch == VK_TAB) { isDateInput = !isDateInput; return 0; }

            if (!isDateInput) {
                if (ch == VK_BACK) { if (!data->name.empty()) data->name.resize(data->name.size() - 1); }
                else if (ch >= 32 && data->name.size() < 20) data->name += ch;
            } else {
                if (ch == VK_BACK) { if (!data->date.empty()) data->date.resize(data->date.size() - 1); }
                else if ((ch >= '0' && ch <= '9') || ch == '-') {
                    if (data->date.size() < 10) data->date += ch;
                }
            }
            InvalidateRect(hDlg, NULL, TRUE);
            return 0;
        }

        case WM_KEYDOWN: {
            if (wParam == VK_RETURN) { data->result = true; EndDialog(hDlg, 1); return 0; }
            if (wParam == VK_ESCAPE) { data->result = false; EndDialog(hDlg, 0); return 0; }
            return 0;
        }

        case WM_CLOSE:
            data->result = false; EndDialog(hDlg, 0); return 0;
    }
    return FALSE;
}

bool CalendarWindow::ShowAddCountdownDialog() {
    AddCdDlgData data;
    data.result = false;
    data.name = L"";
    data.date = L"2026-06-20";
    data.theme = &g_themes[themeIndex_];

    RECT rc; GetWindowRect(hwnd_, &rc);
    int dlgW = 300, dlgH = 290;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance_;
    wc.lpszClassName = L"AddCdDlgClass";
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hDlg = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        L"AddCdDlgClass", L"",
        WS_POPUP | WS_VISIBLE,
        rc.left + (WINDOW_WIDTH - dlgW) / 2, rc.top + 80,
        dlgW, dlgH,
        hwnd_, NULL, hInstance_, NULL
    );

    if (!hDlg) {
        UnregisterClassW(L"AddCdDlgClass", hInstance_);
        return false;
    }

    SetLayeredWindowAttributes(hDlg, 0, 250, LWA_ALPHA);
    SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)AddCdDlgProc);
    SendMessage(hDlg, WM_INITDIALOG, 0, (LPARAM)&data);

    MSG msg; BOOL ret; bool done = false;
    while (!done && (ret = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (ret == -1) break;
        if (msg.hwnd == hDlg || IsChild(hDlg, msg.hwnd)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!IsWindow(hDlg)) { done = true; break; }
    }

    if (IsWindow(hDlg)) DestroyWindow(hDlg);
    UnregisterClassW(L"AddCdDlgClass", hInstance_);

    if (data.result && !data.name.empty() && data.date.size() >= 10) {
        AddCountdown(data.name, data.date);
        return true;
    }
    return false;
}

// ===== 数据存储 =====
std::wstring CalendarWindow::GetDataFilePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hInstance_, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) dir = dir.substr(0, pos + 1);
    return dir + L"data.txt";
}

static std::string W2U(const std::wstring& w) {
    if (w.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, NULL, 0, NULL, NULL);
    std::string r(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &r[0], len, NULL, NULL);
    return r;
}

static std::wstring U2W(const std::string& s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    std::wstring r(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &r[0], len);
    return r;
}

void CalendarWindow::LoadData() {
    std::wstring path = GetDataFilePath();
    std::ifstream file(W2U(path));
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
            std::string k = line.substr(0, eq), v = line.substr(eq + 1);
            if (k == "theme") themeIndex_ = atoi(v.c_str()) % 4;
            else if (k == "collapsed") isCollapsed_ = (v == "1");
        } else if (section == "Tasks") {
            size_t p1 = line.find('|'); if (p1 == std::string::npos) continue;
            size_t p2 = line.find('|', p1 + 1); if (p2 == std::string::npos) continue;
            size_t p3 = line.find('|', p2 + 1); if (p3 == std::string::npos) continue;
            size_t p4 = line.find('|', p3 + 1);
            Task t;
            t.id = atoi(line.substr(0, p1).c_str());
            t.completed = (line.substr(p1 + 1, p2 - p1 - 1) == "1");
            t.repeatType = (RepeatType)atoi(line.substr(p2 + 1, p3 - p2 - 1).c_str());
            if (p4 != std::string::npos) {
                t.customWeekdays = atoi(line.substr(p3 + 1, p4 - p3 - 1).c_str());
                t.text = U2W(line.substr(p4 + 1));
            } else {
                t.customWeekdays = 0;
                t.text = U2W(line.substr(p3 + 1));
            }
            t.lastCompletedDate = L"";
            tasks_.push_back(t);
        } else if (section == "Countdowns") {
            size_t p1 = line.find('|'); if (p1 == std::string::npos) continue;
            size_t p2 = line.find('|', p1 + 1); if (p2 == std::string::npos) continue;
            Countdown c;
            c.id = atoi(line.substr(0, p1).c_str());
            c.name = U2W(line.substr(p1 + 1, p2 - p1 - 1));
            c.targetDate = U2W(line.substr(p2 + 1));
            countdowns_.push_back(c);
        }
    }
    file.close();
}

void CalendarWindow::SaveData() {
    std::wstring path = GetDataFilePath();
    std::ofstream file(W2U(path));
    if (!file.is_open()) return;

    file << "[Settings]\n";
    file << "theme=" << themeIndex_ << "\n";
    file << "collapsed=" << (isCollapsed_ ? "1" : "0") << "\n\n";

    file << "[Tasks]\n";
    for (const auto& t : tasks_) {
        file << t.id << "|"
             << (t.completed ? "1" : "0") << "|"
             << (int)t.repeatType << "|"
             << t.customWeekdays << "|"
             << W2U(t.text) << "\n";
    }
    file << "\n";

    file << "[Countdowns]\n";
    for (const auto& c : countdowns_) {
        file << c.id << "|" << W2U(c.name) << "|" << W2U(c.targetDate) << "\n";
    }
    file << "\n";
    file.close();
}
