#pragma once

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <string>
#include <vector>

namespace utils {

std::wstring Utf8ToWide(const std::string& utf8);
std::string WideToUtf8(const std::wstring& wide);

std::wstring IntToWStr(int value);
int WStrToInt(const std::wstring& str);

std::wstring GetCurrentDateStr();
std::wstring FormatDateMD(const std::wstring& dateStr);

void DrawRoundRect(HDC hdc, int left, int top, int right, int bottom, int radius);
void FillRoundRect(HDC hdc, int left, int top, int right, int bottom, int radius, COLORREF color);
void DrawTextCentered(HDC hdc, const std::wstring& text, RECT rect, COLORREF color, HFONT font);
void DrawTextLeft(HDC hdc, const std::wstring& text, RECT rect, COLORREF color, HFONT font);

HFONT CreateFontSimple(int height, int weight, const wchar_t* faceName, bool italic = false);

COLORREF ARGBToCOLORREF(BYTE a, BYTE r, BYTE g, BYTE b);

int InputBoxW(HWND hwndParent, const wchar_t* prompt, const wchar_t* title, wchar_t* buffer, int bufferSize);

}
