#pragma once
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <string>
#include <vector>

enum class RepeatType {
    None = 0,
    Daily = 1,
    Weekdays = 2,
    Weekends = 3,
    Custom = 4
};

struct Task {
    int id;
    std::wstring text;
    bool completed;
    RepeatType repeatType;
    int customWeekdays; 
    std::wstring lastCompletedDate;
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
    COLORREF accentLight;
    COLORREF textPrimary;
    COLORREF textSecondary;
    COLORREF border;
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
    std::wstring GetTodayStr();
    bool IsTaskVisibleToday(const Task& t);
    std::wstring GetRepeatText(const Task& t);

    void AddTask(const std::wstring& text, RepeatType rt, int customDays);
    void ToggleTask(int id);
    void DeleteTask(int id);

    void AddCountdown(const std::wstring& name, const std::wstring& targetDate);
    void DeleteCountdown(int id);

    bool ShowAddTaskDialog();
    bool ShowAddCountdownDialog();

    HWND hwnd_;
    HINSTANCE hInstance_;
    bool isDragging_;
    POINT dragOffset_;
    bool isCollapsed_;
    std::vector<Task> tasks_;
    std::vector<Countdown> countdowns_;
    int themeIndex_;

    static const int WINDOW_WIDTH = 360;
    static const int WINDOW_HEIGHT_EXPANDED = 600;
    static const int WINDOW_HEIGHT_COLLAPSED = 140;
};
