#pragma once

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <string>
#include <vector>

struct Task {
    int id;
    std::wstring text;
    bool completed;
};

struct Countdown {
    int id;
    std::wstring name;
    std::wstring targetDate;
};

struct ThemeColors {
    COLORREF bgPrimary;
    COLORREF bgSecondary;
    COLORREF accent;
    COLORREF accentDark;
    COLORREF textPrimary;
    COLORREF textSecondary;
    std::wstring seasonText;
};

class CalendarWindow {
public:
    CalendarWindow();
    ~CalendarWindow();

    bool Create(HINSTANCE hInstance);
    void Show(int nCmdShow);

private:
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    void OnPaint();
    void OnLButtonDown(int x, int y);
    void OnLButtonUp();
    void OnMouseMove(int x, int y);
    void OnRButtonUp(int x, int y);

    void PaintBackground(HDC hdc);
    void PaintHeader(HDC hdc);
    void PaintCalendar(HDC hdc);
    void PaintTasks(HDC hdc);
    void PaintCountdowns(HDC hdc);
    void PaintFooter(HDC hdc);

    void LoadData();
    void SaveData();
    std::wstring GetDataFilePath();

    void ToggleCollapse();
    void NextTheme();

    int CalculateDaysLeft(const std::wstring& targetDate);
    std::wstring GetWeekdayName();

    void AddTask(const std::wstring& text);
    void ToggleTask(int id);

    void AddCountdown(const std::wstring& name, const std::wstring& targetDate);

    bool ShowInputDialog(const wchar_t* title, const wchar_t* prompt, std::wstring& result);
    bool ShowAddCountdownDialog(std::wstring& name, std::wstring& date);

    HWND hwnd_;
    HINSTANCE hInstance_;

    bool isDragging_;
    POINT dragOffset_;

    bool isCollapsed_;

    std::vector<Task> tasks_;
    std::vector<Countdown> countdowns_;

    int themeIndex_;

    static const int WINDOW_WIDTH = 340;
    static const int WINDOW_HEIGHT_EXPANDED = 560;
    static const int WINDOW_HEIGHT_COLLAPSED = 130;
};
