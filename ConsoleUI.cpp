#include "pch.h"
#include "ConsoleUI.h"

namespace myutils
{
    namespace ui
    {
        static HANDLE g_out = nullptr;

        static void setSize(int w, int h)
        {
            SMALL_RECT tiny = { 0, 0, 1, 1 };
            SetConsoleWindowInfo(g_out, TRUE, &tiny);

            COORD buf = { (SHORT)w, (SHORT)h };
            SetConsoleScreenBufferSize(g_out, buf);

            SMALL_RECT rect = { 0, 0, (SHORT)(w - 1), (SHORT)(h - 1) };
            SetConsoleWindowInfo(g_out, TRUE, &rect);
        }

        void init()
        {
            g_out = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleOutputCP(CP_UTF8);

            DWORD mode = 0;
            if (GetConsoleMode(g_out, &mode))
                SetConsoleMode(g_out, mode & ~ENABLE_WRAP_AT_EOL_OUTPUT);

            setSize(SCREEN_W, SCREEN_H);
            hideCursor();
        }

        void hideCursor()
        {
            CONSOLE_CURSOR_INFO ci{};
            GetConsoleCursorInfo(g_out, &ci);
            ci.bVisible = FALSE;
            SetConsoleCursorInfo(g_out, &ci);
        }

        void showCursor()
        {
            CONSOLE_CURSOR_INFO ci{};
            GetConsoleCursorInfo(g_out, &ci);
            ci.bVisible = TRUE;
            SetConsoleCursorInfo(g_out, &ci);
        }

        void gotoxy(int x, int y)
        {
            COORD c = { (SHORT)x, (SHORT)y };
            SetConsoleCursorPosition(g_out, c);
        }

        void setColor(WORD attr)
        {
            SetConsoleTextAttribute(g_out, attr);
        }

        void clear(WORD attr)
        {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (!GetConsoleScreenBufferInfo(g_out, &csbi)) return;


            DWORD cells = (DWORD)csbi.dwSize.X * (DWORD)csbi.dwSize.Y;
            COORD home = { 0, 0 };
            DWORD written = 0;

            FillConsoleOutputCharacterW(g_out, L' ', cells, home, &written);
            FillConsoleOutputAttribute(g_out, attr, cells, home, &written);
            gotoxy(0, 0);
        }

        void clearRow(int y, WORD attr)
        {
            COORD at = { 0, (SHORT)y };
            DWORD written = 0;

            // easiest way to do this, stack overflow I love you.
            FillConsoleOutputCharacterW(g_out, L' ', SCREEN_W, at, &written);
            FillConsoleOutputAttribute(g_out, attr, SCREEN_W, at, &written);
        }

        void putString(int x, int y, const std::wstring& s, WORD attr)
        {
            setColor(attr);
            gotoxy(x, y);
            DWORD written = 0;
            WriteConsoleW(g_out, s.c_str(), (DWORD)s.size(), &written, nullptr);
        }

        void writeRaw(const std::wstring& s, WORD attr)
        {
            setColor(attr);
            DWORD written = 0;
            WriteConsoleW(g_out, s.c_str(), (DWORD)s.size(), &written, nullptr);
        }

        std::wstring fit(const std::wstring& s, int width)
        {
            if (width <= 0) return L"";
            if ((int)s.size() <= width) return s;
            if (width == 1) return std::wstring(1, L'\u2026');
            return s.substr(0, width - 1) + L"\u2026";
        }

        void putBar(int x, int y, int width, const std::wstring& label, WORD attr, int padLeft)
        {
            std::wstring bar(width, L' ');
            std::wstring lab = fit(label, width - padLeft);
            for (size_t i = 0; i < lab.size(); ++i)
                bar[padLeft + i] = lab[i];
            putString(x, y, bar, attr);
        }

        void putBarCentered(int x, int y, int width, const std::wstring& label, WORD attr)
        {
            std::wstring bar(width, L' ');
            std::wstring lab = fit(label, width);
            int start = (width - (int)lab.size()) / 2;
            if (start < 0) start = 0;
            for (size_t i = 0; i < lab.size() && start + (int)i < width; ++i)
                bar[start + i] = lab[i];
            putString(x, y, bar, attr);
        }
    }
}