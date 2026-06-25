#include "themes.h"

namespace themes {

ThemeColors GetSpringTheme() {
    ThemeColors t;
    t.bgPrimary = RGB(240, 248, 240);
    t.bgSecondary = RGB(220, 240, 220);
    t.accent = RGB(144, 205, 144);
    t.accentDark = RGB(107, 172, 107);
    t.textPrimary = RGB(46, 125, 50);
    t.textSecondary = RGB(85, 139, 47);
    t.border = RGB(165, 214, 167);
    t.alpha = 225;
    t.seasonText = L"樱落时节";
    return t;
}

ThemeColors GetSummerTheme() {
    ThemeColors t;
    t.bgPrimary = RGB(232, 245, 233);
    t.bgSecondary = RGB(200, 230, 201);
    t.accent = RGB(129, 199, 132);
    t.accentDark = RGB(102, 187, 106);
    t.textPrimary = RGB(46, 125, 50);
    t.textSecondary = RGB(85, 139, 47);
    t.border = RGB(165, 214, 167);
    t.alpha = 225;
    t.seasonText = L"蝉鸣之夏";
    return t;
}

ThemeColors GetAutumnTheme() {
    ThemeColors t;
    t.bgPrimary = RGB(255, 243, 224);
    t.bgSecondary = RGB(255, 224, 178);
    t.accent = RGB(255, 167, 38);
    t.accentDark = RGB(255, 143, 0);
    t.textPrimary = RGB(191, 93, 0);
    t.textSecondary = RGB(230, 109, 0);
    t.border = RGB(255, 183, 77);
    t.alpha = 225;
    t.seasonText = L"枫红秋意";
    return t;
}

ThemeColors GetWinterTheme() {
    ThemeColors t;
    t.bgPrimary = RGB(227, 242, 253);
    t.bgSecondary = RGB(187, 222, 251);
    t.accent = RGB(100, 181, 246);
    t.accentDark = RGB(66, 165, 245);
    t.textPrimary = RGB(25, 118, 210);
    t.textSecondary = RGB(57, 139, 247);
    t.border = RGB(144, 202, 249);
    t.alpha = 225;
    t.seasonText = L"雪落冬安";
    return t;
}

ThemeColors GetThemeBySeason(Season season) {
    switch (season) {
        case Season::Spring: return GetSpringTheme();
        case Season::Summer: return GetSummerTheme();
        case Season::Autumn: return GetAutumnTheme();
        case Season::Winter: return GetWinterTheme();
    }
    return GetSummerTheme();
}

ThemeColors GetThemeByIndex(int index) {
    switch (index % 4) {
        case 0: return GetSpringTheme();
        case 1: return GetSummerTheme();
        case 2: return GetAutumnTheme();
        case 3: return GetWinterTheme();
    }
    return GetSummerTheme();
}

int GetThemeCount() {
    return 4;
}

}
