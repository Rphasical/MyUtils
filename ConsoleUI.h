#pragma once
#include "pch.h"

namespace myutils
{
    namespace ui
    {

        constexpr int SCREEN_W = 80;
        constexpr int SCREEN_H = 25;

        constexpr WORD BG = 0;
        constexpr WORD BORDER = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        constexpr WORD TEXT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        constexpr WORD TITLE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        constexpr WORD ACCENT = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        constexpr WORD DIM = FOREGROUND_INTENSITY;
        constexpr WORD SELECTED = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
        constexpr WORD BARBG = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        constexpr WORD NORMAL = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        constexpr int KEY_RESIZE = -2;
        constexpr int KEY_EXT = 0x100;

        void init();
        void applySize();
        int  readKey();
        void hideCursor();
        void showCursor();
        void clearRow(int y, WORD attr = BG);
        void clear(WORD attr = BG);
        void fullClear(WORD attr = BG);
        void getSize(int& w, int& h);
        void gotoxy(int x, int y);
        void setColor(WORD attr);
        void putString(int x, int y, const std::wstring& s, WORD attr);
        void putBar(int x, int y, int width, const std::wstring& label, WORD attr, int padLeft = 1);
        void putBarCentered(int x, int y, int width, const std::wstring& label, WORD attr);
        void writeRaw(const std::wstring& s, WORD attr);

        std::wstring fit(const std::wstring& s, int width);
    }
}