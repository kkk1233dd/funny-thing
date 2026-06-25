#pragma once

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <string>
#include <vector>
#include <memory>

struct Task {
    int id;
    std::wstring text;
    bool completed;
    std::wstring createdAt;
};

struct Countdown {
    int id;
    std::wstring name;
    std::wstring targetDate; // YYYY-MM-DD
    std::wstring createdAt;
};

enum class Season {
    Spring,
    Summer,
    Autumn,
    Winter
};

struct ThemeColors {
    COLORREF bgPrimary;
    COLORREF bgSecondary;
    COLORREF accent;
    COLORREF accentDark;
    COLORREF textPrimary;
    COLORREF textSecondary;
    COLORREF border;
    BYTE alpha;
    std::wstring seasonText;
};

class DesktopCalendarWindow {
public:
    DesktopCalendarWindow();
    ~DesktopCalendarWindow();

    bool Create(HINSTANCE hInstance);
    void Show(int nCmdShow);

private:
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    void OnPaint(HDC hdc);
    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);
    void OnMouseMove(int x, int y);
    void OnRButtonUp(int x, int y);
    void OnCommand(int id, HWND hCtl, UINT codeNotify);

    void PaintBackground(HDC hdc);
    void PaintCalendar(HDC hdc);
    void PaintTasks(HDC hdc);
    void PaintCountdowns(HDC hdc);
    void PaintFooter(HDC hdc);
    void PaintCollapseButton(HDC hdc);

    void LoadData();
    void SaveData();
    std::wstring GetDataFilePath();

    void UpdateSeason();
    void ToggleCollapse();
    void NextTheme();

    int CalculateDaysLeft(const std::wstring& targetDate);
    std::wstring GetLunarDate();
    std::wstring GetWeekdayName();
    Season GetCurrentSeason();

    void AddTask(const std::wstring& text);
    void ToggleTask(int id);
    void DeleteTask(int id);

    void AddCountdown(const std::wstring& name, const std::wstring& targetDate);
    void DeleteCountdown(int id);

    void ShowAddTaskDialog();
    void ShowAddCountdownDialog();

    RECT GetClientRect();

    HWND hwnd_;
    HINSTANCE hInstance_;

    bool isDragging_;
    POINT dragOffset_;

    bool isCollapsed_;

    std::vector<Task> tasks_;
    std::vector<Countdown> countdowns_;

    ThemeColors currentTheme_;
    int themeIndex_;

    int hoverItemId_;
    bool isTaskHover_;

    static const int WINDOW_WIDTH = 340;
    static const int WINDOW_HEIGHT_EXPANDED = 560;
    static const int WINDOW_HEIGHT_COLLAPSED = 120;
    static const int HEADER_HEIGHT = 48;
    static const int CALENDAR_HEIGHT = 120;
    static const int FOOTER_HEIGHT = 40;
};
