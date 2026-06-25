#include "window.h"
#include "themes.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <time.h>

#define ID_ADD_TASK 1001
#define ID_ADD_COUNTDOWN 1002
#define ID_TOGGLE_TASK 1003
#define ID_DELETE_TASK 1004
#define ID_DELETE_COUNTDOWN 1005
#define ID_TOGGLE_COLLAPSE 1006
#define ID_NEXT_THEME 1007
#define ID_EXIT 1008

DesktopCalendarWindow::DesktopCalendarWindow()
    : hwnd_(NULL)
    , hInstance_(NULL)
    , isDragging_(false)
    , isCollapsed_(false)
    , themeIndex_(1)
    , hoverItemId_(-1)
    , isTaskHover_(true)
{
    currentTheme_ = themes::GetThemeByIndex(themeIndex_);
}

DesktopCalendarWindow::~DesktopCalendarWindow() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
    }
}

bool DesktopCalendarWindow::Create(HINSTANCE hInstance) {
    hInstance_ = hInstance;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProcStatic;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DesktopCalendarWidget";
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int height = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        L"DesktopCalendarWidget",
        L"桌面日历",
        WS_POPUP | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, height,
        NULL, NULL, hInstance, this
    );

    if (!hwnd_) return false;

    SetLayeredWindowAttributes(hwnd_, 0, currentTheme_.alpha, LWA_ALPHA);

    MARGINS margins = {-1};
    DwmExtendFrameIntoClientArea(hwnd_, &margins);

    LoadData();
    UpdateSeason();

    return true;
}

void DesktopCalendarWindow::Show(int nCmdShow) {
    ShowWindow(hwnd_, nCmdShow);
    UpdateWindow(hwnd_);
}

LRESULT CALLBACK DesktopCalendarWindow::WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    DesktopCalendarWindow* pThis = NULL;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        pThis = (DesktopCalendarWindow*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->hwnd_ = hwnd;
    } else {
        pThis = (DesktopCalendarWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->WndProc(msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT DesktopCalendarWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd_, &ps);
            OnPaint(hdc);
            EndPaint(hwnd_, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            if (x >= 8 && x <= 32 && y >= 12 && y <= 36) {
                ToggleCollapse();
                return 0;
            }

            OnLButtonDown(x, y);
            return 0;
        }

        case WM_LBUTTONUP:
            OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_MOUSEMOVE:
            OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_RBUTTONUP:
            OnRButtonUp(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_COMMAND:
            OnCommand(LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
            return 0;

        case WM_TIMER:
            InvalidateRect(hwnd_, NULL, FALSE);
            return 0;

        case WM_DESTROY:
            SaveData();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd_, msg, wParam, lParam);
    }
}

void DesktopCalendarWindow::OnPaint(HDC hdc) {
    RECT rc;
    ::GetClientRect(hwnd_, &rc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

    PaintBackground(memDC);
    PaintCollapseButton(memDC);
    PaintCalendar(memDC);

    if (!isCollapsed_) {
        PaintTasks(memDC);
        PaintCountdowns(memDC);
    }

    PaintFooter(memDC);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

void DesktopCalendarWindow::PaintBackground(HDC hdc) {
    RECT rc;
    ::GetClientRect(hwnd_, &rc);

    HBRUSH hBrush = CreateSolidBrush(currentTheme_.bgPrimary);
    FillRect(hdc, &rc, hBrush);
    DeleteObject(hBrush);

    HRGN hRgn = CreateRoundRectRgn(0, 0, rc.right, rc.bottom, 24, 24);
    FillRgn(hdc, hRgn, (HBRUSH)CreateSolidBrush(currentTheme_.bgPrimary));
    DeleteObject(hRgn);
}

void DesktopCalendarWindow::PaintCollapseButton(HDC hdc) {
    const wchar_t* symbol = isCollapsed_ ? L"≡" : L"≡";
    RECT rc = {8, 12, 32, 36};

    HFONT font = utils::CreateFontSimple(18, FW_BOLD, L"Microsoft YaHei UI");
    utils::DrawTextCentered(hdc, symbol, rc, currentTheme_.accentDark, font);
    DeleteObject(font);
}

void DesktopCalendarWindow::PaintCalendar(HDC hdc) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    int top = HEADER_HEIGHT;
    int bottom = top + CALENDAR_HEIGHT;

    RECT titleRect = {0, top - 10, WINDOW_WIDTH, top + 10};
    HFONT titleFont = utils::CreateFontSimple(14, FW_MEDIUM, L"Microsoft YaHei UI");
    utils::DrawTextCentered(hdc, L"✨ 每日任务", titleRect, currentTheme_.textPrimary, titleFont);
    DeleteObject(titleFont);

    if (!isCollapsed_) {
        std::wstring dayStr = utils::IntToWStr(st.wDay);
        RECT dayRect = {40, top + 20, 110, top + 80};
        HFONT dayFont = utils::CreateFontSimple(56, FW_LIGHT, L"Microsoft YaHei UI");
        utils::DrawTextCentered(hdc, dayStr, dayRect, currentTheme_.textPrimary, dayFont);
        DeleteObject(dayFont);

        std::wstring weekdayName = GetWeekdayName();
        std::wstring monthStr = utils::IntToWStr(st.wYear) + L"年" + utils::IntToWStr(st.wMonth) + L"月";

        RECT infoRect = {115, top + 25, 220, top + 50};
        HFONT weekFont = utils::CreateFontSimple(14, FW_MEDIUM, L"Microsoft YaHei UI");
        utils::DrawTextLeft(hdc, weekdayName, infoRect, currentTheme_.textSecondary, weekFont);
        DeleteObject(weekFont);

        RECT monthRect = {115, top + 50, 220, top + 75};
        HFONT monthFont = utils::CreateFontSimple(11, FW_NORMAL, L"Microsoft YaHei UI");
        utils::DrawTextLeft(hdc, monthStr, monthRect, currentTheme_.accent, monthFont);
        DeleteObject(monthFont);

        std::wstring lunar = GetLunarDate();
        RECT lunarRect = {80, top + 85, 200, top + 105};
        HFONT lunarFont = utils::CreateFontSimple(11, FW_NORMAL, L"Microsoft YaHei UI");
        utils::DrawTextCentered(hdc, lunar, lunarRect, currentTheme_.accentDark, lunarFont);
        DeleteObject(lunarFont);
    }
}

void DesktopCalendarWindow::PaintTasks(HDC hdc) {
    int y = HEADER_HEIGHT + CALENDAR_HEIGHT + 10;

    RECT headerRect = {20, y, WINDOW_WIDTH - 20, y + 24};
    HFONT sectionFont = utils::CreateFontSimple(12, FW_MEDIUM, L"Microsoft YaHei UI");
    utils::DrawTextLeft(hdc, L"📝 今日任务", headerRect, currentTheme_.textPrimary, sectionFont);
    DeleteObject(sectionFont);

    RECT addRect = {WINDOW_WIDTH - 44, y, WINDOW_WIDTH - 20, y + 24};
    HFONT addFont = utils::CreateFontSimple(18, FW_NORMAL, L"Microsoft YaHei UI");
    utils::DrawTextCentered(hdc, L"+", addRect, currentTheme_.accentDark, addFont);
    DeleteObject(addFont);

    y += 30;

    if (tasks_.empty()) {
        RECT emptyRect = {20, y, WINDOW_WIDTH - 20, y + 40};
        HFONT emptyFont = utils::CreateFontSimple(11, FW_NORMAL, L"Microsoft YaHei UI");
        utils::DrawTextCentered(hdc, L"还没有任务，点击右上角 + 添加", emptyRect, RGB(180, 180, 180), emptyFont);
        DeleteObject(emptyFont);
        y += 50;
    } else {
        HFONT taskFont = utils::CreateFontSimple(12, FW_NORMAL, L"Microsoft YaHei UI");
        int i = 0;
        for (const auto& task : tasks_) {
            if (i >= 4) break;

            int itemY = y + i * 36;
            RECT itemRect = {20, itemY, WINDOW_WIDTH - 20, itemY + 32};

            COLORREF bg = task.completed ? RGB(220, 220, 220) : RGB(255, 255, 255);
            utils::FillRoundRect(hdc, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom, 8, bg);

            RECT checkRect = {28, itemY + 6, 48, itemY + 26};
            if (task.completed) {
                HBRUSH hBrush = CreateSolidBrush(currentTheme_.accent);
                FillRect(hdc, &checkRect, hBrush);
                DeleteObject(hBrush);

                HFONT checkFont = utils::CreateFontSimple(14, FW_BOLD, L"Microsoft YaHei UI");
                utils::DrawTextCentered(hdc, L"✓", checkRect, RGB(255,255,255), checkFont);
                DeleteObject(checkFont);
            } else {
                RECT frameRect = {checkRect.left + 2, checkRect.top + 2, checkRect.right - 2, checkRect.bottom - 2};
                HBRUSH hBrush = CreateSolidBrush(RGB(255,255,255));
                HPEN hPen = CreatePen(PS_SOLID, 2, currentTheme_.accent);
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                Rectangle(hdc, frameRect.left, frameRect.top, frameRect.right, frameRect.bottom);
                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(hBrush);
                DeleteObject(hPen);
            }

            RECT textRect = {58, itemY + 4, WINDOW_WIDTH - 60, itemY + 28};
            COLORREF textColor = task.completed ? RGB(160, 160, 160) : currentTheme_.textPrimary;
            utils::DrawTextLeft(hdc, task.text, textRect, textColor, taskFont);

            i++;
        }
        DeleteObject(taskFont);
        y += std::min((int)tasks_.size(), 4) * 36 + 10;
    }
}

void DesktopCalendarWindow::PaintCountdowns(HDC hdc) {
    int y = HEADER_HEIGHT + CALENDAR_HEIGHT + 180;
    if (tasks_.empty()) y = HEADER_HEIGHT + CALENDAR_HEIGHT + 140;

    RECT headerRect = {20, y, WINDOW_WIDTH - 20, y + 24};
    HFONT sectionFont = utils::CreateFontSimple(12, FW_MEDIUM, L"Microsoft YaHei UI");
    utils::DrawTextLeft(hdc, L"⏰ 倒计时", headerRect, currentTheme_.textPrimary, sectionFont);
    DeleteObject(sectionFont);

    RECT addRect = {WINDOW_WIDTH - 44, y, WINDOW_WIDTH - 20, y + 24};
    HFONT addFont = utils::CreateFontSimple(18, FW_NORMAL, L"Microsoft YaHei UI");
    utils::DrawTextCentered(hdc, L"+", addRect, currentTheme_.accentDark, addFont);
    DeleteObject(addFont);

    y += 30;

    if (countdowns_.empty()) {
        RECT emptyRect = {20, y, WINDOW_WIDTH - 20, y + 40};
        HFONT emptyFont = utils::CreateFontSimple(11, FW_NORMAL, L"Microsoft YaHei UI");
        utils::DrawTextCentered(hdc, L"还没有倒计时，点击右上角 + 添加", emptyRect, RGB(180, 180, 180), emptyFont);
        DeleteObject(emptyFont);
    } else {
        HFONT nameFont = utils::CreateFontSimple(13, FW_MEDIUM, L"Microsoft YaHei UI");
        HFONT dateFont = utils::CreateFontSimple(10, FW_NORMAL, L"Microsoft YaHei UI");
        HFONT numFont = utils::CreateFontSimple(22, FW_LIGHT, L"Microsoft YaHei UI");
        HFONT unitFont = utils::CreateFontSimple(9, FW_NORMAL, L"Microsoft YaHei UI");

        int i = 0;
        for (const auto& cd : countdowns_) {
            if (i >= 3) break;

            int itemY = y + i * 44;
            RECT itemRect = {20, itemY, WINDOW_WIDTH - 20, itemY + 40};
            utils::FillRoundRect(hdc, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom, 10, currentTheme_.bgSecondary);

            RECT iconRect = {28, itemY + 6, 60, itemY + 34};
            utils::DrawTextCentered(hdc, L"⏰", iconRect, currentTheme_.accentDark, nameFont);

            RECT nameRect = {68, itemY + 4, 200, itemY + 20};
            utils::DrawTextLeft(hdc, cd.name, nameRect, currentTheme_.textPrimary, nameFont);

            RECT dateRect = {68, itemY + 20, 200, itemY + 34};
            std::wstring dateStr = utils::FormatDateMD(cd.targetDate);
            utils::DrawTextLeft(hdc, dateStr, dateRect, RGB(150, 150, 150), dateFont);

            int days = CalculateDaysLeft(cd.targetDate);
            RECT numRect = {WINDOW_WIDTH - 80, itemY + 2, WINDOW_WIDTH - 40, itemY + 28};
            utils::DrawTextCentered(hdc, utils::IntToWStr(days), numRect, currentTheme_.accentDark, numFont);

            RECT unitRect = {WINDOW_WIDTH - 80, itemY + 26, WINDOW_WIDTH - 40, itemY + 38};
            utils::DrawTextCentered(hdc, L"天", unitRect, currentTheme_.accent, unitFont);

            i++;
        }
        DeleteObject(nameFont);
        DeleteObject(dateFont);
        DeleteObject(numFont);
        DeleteObject(unitFont);
    }
}

void DesktopCalendarWindow::PaintFooter(HDC hdc) {
    int bottom = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;

    RECT footerRect = {0, bottom - FOOTER_HEIGHT, WINDOW_WIDTH, bottom};
    HFONT footerFont = utils::CreateFontSimple(10, FW_NORMAL, L"Microsoft YaHei UI");

    std::wstring footerText = L"━━━ " + currentTheme_.seasonText + L" ━━━";
    utils::DrawTextCentered(hdc, footerText, footerRect, currentTheme_.accent, footerFont);

    DeleteObject(footerFont);
}

void DesktopCalendarWindow::OnLButtonDown(int x, int y) {
    if (y < HEADER_HEIGHT) {
        isDragging_ = true;
        POINT pt;
        GetCursorPos(&pt);
        RECT rc;
        GetWindowRect(hwnd_, &rc);
        dragOffset_.x = pt.x - rc.left;
        dragOffset_.y = pt.y - rc.top;
        SetCapture(hwnd_);
    }

    if (!isCollapsed_) {
        int taskY = HEADER_HEIGHT + CALENDAR_HEIGHT + 10;
        int addBtnY = taskY;
        if (x >= WINDOW_WIDTH - 44 && x <= WINDOW_WIDTH - 20 &&
            y >= addBtnY && y <= addBtnY + 24) {
            ShowAddTaskDialog();
            return;
        }

        int cdY = HEADER_HEIGHT + CALENDAR_HEIGHT + 180;
        if (tasks_.empty()) cdY = HEADER_HEIGHT + CALENDAR_HEIGHT + 140;
        int cdAddBtnY = cdY;
        if (x >= WINDOW_WIDTH - 44 && x <= WINDOW_WIDTH - 20 &&
            y >= cdAddBtnY && y <= cdAddBtnY + 24) {
            ShowAddCountdownDialog();
            return;
        }

        int taskListY = taskY + 30;
        for (size_t i = 0; i < tasks_.size() && i < 4; i++) {
            int itemY = taskListY + (int)i * 36;
            if (y >= itemY && y <= itemY + 32) {
                if (x >= 28 && x <= 48) {
                    ToggleTask(tasks_[i].id);
                    return;
                }
            }
        }
    }
}

void DesktopCalendarWindow::OnLButtonUp(int x, int y) {
    if (isDragging_) {
        isDragging_ = false;
        ReleaseCapture();
    }
}

void DesktopCalendarWindow::OnMouseMove(int x, int y) {
    if (isDragging_) {
        POINT pt;
        GetCursorPos(&pt);
        SetWindowPos(hwnd_, NULL,
            pt.x - dragOffset_.x,
            pt.y - dragOffset_.y,
            0, 0,
            SWP_NOSIZE | SWP_NOZORDER);
    }
}

void DesktopCalendarWindow::OnRButtonUp(int x, int y) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TOGGLE_COLLAPSE, isCollapsed_ ? L"展开" : L"折叠");
    AppendMenuW(hMenu, MF_STRING, ID_NEXT_THEME, L"切换主题");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_EXIT, L"退出");

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd_, NULL);
    DestroyMenu(hMenu);
}

void DesktopCalendarWindow::OnCommand(int id, HWND hCtl, UINT codeNotify) {
    switch (id) {
        case ID_TOGGLE_COLLAPSE:
            ToggleCollapse();
            break;
        case ID_NEXT_THEME:
            NextTheme();
            break;
        case ID_EXIT:
            DestroyWindow(hwnd_);
            break;
    }
}

void DesktopCalendarWindow::ToggleCollapse() {
    isCollapsed_ = !isCollapsed_;
    int height = isCollapsed_ ? WINDOW_HEIGHT_COLLAPSED : WINDOW_HEIGHT_EXPANDED;
    SetWindowPos(hwnd_, NULL, 0, 0, WINDOW_WIDTH, height, SWP_NOMOVE | SWP_NOZORDER);
    InvalidateRect(hwnd_, NULL, TRUE);
    SaveData();
}

void DesktopCalendarWindow::NextTheme() {
    themeIndex_ = (themeIndex_ + 1) % themes::GetThemeCount();
    currentTheme_ = themes::GetThemeByIndex(themeIndex_);
    SetLayeredWindowAttributes(hwnd_, 0, currentTheme_.alpha, LWA_ALPHA);
    InvalidateRect(hwnd_, NULL, TRUE);
    SaveData();
}

void DesktopCalendarWindow::UpdateSeason() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    int month = st.wMonth;

    if (month >= 3 && month <= 5) {
        currentTheme_ = themes::GetSpringTheme();
        themeIndex_ = 0;
    } else if (month >= 6 && month <= 8) {
        currentTheme_ = themes::GetSummerTheme();
        themeIndex_ = 1;
    } else if (month >= 9 && month <= 11) {
        currentTheme_ = themes::GetAutumnTheme();
        themeIndex_ = 2;
    } else {
        currentTheme_ = themes::GetWinterTheme();
        themeIndex_ = 3;
    }
}

Season DesktopCalendarWindow::GetCurrentSeason() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    int month = st.wMonth;
    if (month >= 3 && month <= 5) return Season::Spring;
    if (month >= 6 && month <= 8) return Season::Summer;
    if (month >= 9 && month <= 11) return Season::Autumn;
    return Season::Winter;
}

std::wstring DesktopCalendarWindow::GetWeekdayName() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    const wchar_t* days[] = {L"星期日", L"星期一", L"星期二", L"星期三", L"星期四", L"星期五", L"星期六"};
    return days[st.wDayOfWeek];
}

std::wstring DesktopCalendarWindow::GetLunarDate() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    const wchar_t* lunarMonths[] = {L"正", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九", L"十", L"冬", L"腊"};
    const wchar_t* lunarDays[] = {
        L"初一", L"初二", L"初三", L"初四", L"初五", L"初六", L"初七", L"初八", L"初九", L"初十",
        L"十一", L"十二", L"十三", L"十四", L"十五", L"十六", L"十七", L"十八", L"十九", L"二十",
        L"廿一", L"廿二", L"廿三", L"廿四", L"廿五", L"廿六", L"廿七", L"廿八", L"廿九", L"三十"
    };

    int lunarDay = (st.wDay + 10) % 30;
    if (lunarDay == 0) lunarDay = 30;
    int lunarMonth = (st.wMonth + 10) % 12;

    return std::wstring(L"农历") + lunarMonths[lunarMonth] + L"月" + lunarDays[lunarDay - 1];
}

int DesktopCalendarWindow::CalculateDaysLeft(const std::wstring& targetDate) {
    if (targetDate.size() < 10) return 0;

    int year = utils::WStrToInt(targetDate.substr(0, 4));
    int month = utils::WStrToInt(targetDate.substr(5, 2));
    int day = utils::WStrToInt(targetDate.substr(8, 2));

    SYSTEMTIME st;
    GetLocalTime(&st);

    struct tm now_tm = {0};
    now_tm.tm_year = st.wYear - 1900;
    now_tm.tm_mon = st.wMonth - 1;
    now_tm.tm_mday = st.wDay;

    struct tm target_tm = {0};
    target_tm.tm_year = year - 1900;
    target_tm.tm_mon = month - 1;
    target_tm.tm_mday = day;

    time_t now = mktime(&now_tm);
    time_t target = mktime(&target_tm);

    double diff = difftime(target, now);
    int days = (int)(diff / (60 * 60 * 24)) + 1;
    return days > 0 ? days : 0;
}

void DesktopCalendarWindow::AddTask(const std::wstring& text) {
    if (text.empty()) return;
    Task t;
    t.id = (int)time(NULL);
    t.text = text;
    t.completed = false;
    t.createdAt = utils::GetCurrentDateStr();
    tasks_.push_back(t);
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void DesktopCalendarWindow::ToggleTask(int id) {
    for (auto& t : tasks_) {
        if (t.id == id) {
            t.completed = !t.completed;
            break;
        }
    }
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void DesktopCalendarWindow::DeleteTask(int id) {
    tasks_.erase(std::remove_if(tasks_.begin(), tasks_.end(),
        [id](const Task& t) { return t.id == id; }), tasks_.end());
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void DesktopCalendarWindow::AddCountdown(const std::wstring& name, const std::wstring& targetDate) {
    if (name.empty() || targetDate.empty()) return;
    Countdown c;
    c.id = (int)time(NULL) + rand();
    c.name = name;
    c.targetDate = targetDate;
    c.createdAt = utils::GetCurrentDateStr();
    countdowns_.push_back(c);
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

void DesktopCalendarWindow::DeleteCountdown(int id) {
    countdowns_.erase(std::remove_if(countdowns_.begin(), countdowns_.end(),
        [id](const Countdown& c) { return c.id == id; }), countdowns_.end());
    SaveData();
    InvalidateRect(hwnd_, NULL, TRUE);
}

std::wstring DesktopCalendarWindow::GetDataFilePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hInstance_, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        dir = dir.substr(0, pos + 1);
    }
    return dir + L"data.txt";
}

void DesktopCalendarWindow::LoadData() {
    std::wstring path = GetDataFilePath();
    std::ifstream file(utils::WideToUtf8(path));
    if (!file.is_open()) return;

    std::string line;
    std::string section;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') {
            section = line.substr(1, line.size() - 2);
            continue;
        }

        if (section == "Settings") {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = line.substr(0, eq);
                std::string value = line.substr(eq + 1);
                if (key == "theme") {
                    themeIndex_ = atoi(value.c_str()) % themes::GetThemeCount();
                    currentTheme_ = themes::GetThemeByIndex(themeIndex_);
                    SetLayeredWindowAttributes(hwnd_, 0, currentTheme_.alpha, LWA_ALPHA);
                } else if (key == "collapsed") {
                    isCollapsed_ = (value == "1");
                }
            }
        } else if (section == "Tasks") {
            size_t pos1 = line.find('|');
            if (pos1 != std::string::npos) {
                size_t pos2 = line.find('|', pos1 + 1);
                if (pos2 != std::string::npos) {
                    Task t;
                    t.id = atoi(line.substr(0, pos1).c_str());
                    t.completed = (line.substr(pos1 + 1, pos2 - pos1 - 1) == "1");
                    t.text = utils::Utf8ToWide(line.substr(pos2 + 1));
                    tasks_.push_back(t);
                }
            }
        } else if (section == "Countdowns") {
            size_t pos1 = line.find('|');
            if (pos1 != std::string::npos) {
                size_t pos2 = line.find('|', pos1 + 1);
                if (pos2 != std::string::npos) {
                    Countdown c;
                    c.id = atoi(line.substr(0, pos1).c_str());
                    c.name = utils::Utf8ToWide(line.substr(pos1 + 1, pos2 - pos1 - 1));
                    c.targetDate = utils::Utf8ToWide(line.substr(pos2 + 1));
                    countdowns_.push_back(c);
                }
            }
        }
    }

    file.close();
}

void DesktopCalendarWindow::SaveData() {
    std::wstring path = GetDataFilePath();
    std::ofstream file(utils::WideToUtf8(path));
    if (!file.is_open()) return;

    file << "[Settings]\n";
    file << "theme=" << themeIndex_ << "\n";
    file << "collapsed=" << (isCollapsed_ ? "1" : "0") << "\n";
    file << "\n";

    file << "[Tasks]\n";
    for (const auto& t : tasks_) {
        file << t.id << "|" << (t.completed ? "1" : "0") << "|" << utils::WideToUtf8(t.text) << "\n";
    }
    file << "\n";

    file << "[Countdowns]\n";
    for (const auto& c : countdowns_) {
        file << c.id << "|" << utils::WideToUtf8(c.name) << "|" << utils::WideToUtf8(c.targetDate) << "\n";
    }
    file << "\n";

    file.close();
}

void DesktopCalendarWindow::ShowAddTaskDialog() {
    wchar_t buf[256] = {0};
    if (utils::InputBoxW(hwnd_, L"请输入任务内容：", L"添加任务", buf, 256) > 0) {
        AddTask(buf);
    }
}

void DesktopCalendarWindow::ShowAddCountdownDialog() {
    wchar_t nameBuf[128] = {0};
    wchar_t dateBuf[32] = {0};

    if (utils::InputBoxW(hwnd_, L"请输入事件名称（如：中考）：", L"添加倒计时 - 名称", nameBuf, 128) > 0) {
        wcscpy_s(dateBuf, L"2025-06-20");
        if (utils::InputBoxW(hwnd_, L"请输入目标日期（格式：YYYY-MM-DD）：", L"添加倒计时 - 日期", dateBuf, 32) > 0) {
            AddCountdown(nameBuf, dateBuf);
        }
    }
}

RECT DesktopCalendarWindow::GetClientRect() {
    RECT rc;
    ::GetClientRect(hwnd_, &rc);
    return rc;
}
