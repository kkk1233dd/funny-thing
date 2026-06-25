#pragma once

#include <windows.h>
#include "window.h"

namespace themes {

ThemeColors GetSpringTheme();
ThemeColors GetSummerTheme();
ThemeColors GetAutumnTheme();
ThemeColors GetWinterTheme();

ThemeColors GetThemeBySeason(Season season);
ThemeColors GetThemeByIndex(int index);
int GetThemeCount();

}
