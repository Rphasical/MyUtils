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

            // Pin the buffer to the window so nothing scrolls off and bleeds back lie kthe yucky terminal column lines.
            SetConsoleScreenBufferSize(g_out, buf);
        }

        void init()
        {
            g_out = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleOutputCP(CP_UTF8);

            DWORD mode = 0;
            if (GetConsoleMode(g_out, &mode))
                SetConsoleMode(g_out, mode & ~ENABLE_WRAP_AT_EOL_OUTPUT);

            setSize(SCREEN_W, SCREEN_H);
            fullClear(BG);
            hideCursor();
        }

        void applySize()
        {
            setSize(SCREEN_W, SCREEN_H);
        }

        int readKey()
        {
            static int lastW = -1, lastH = -1;
            if (lastW < 0) getSize(lastW, lastH);

            for (;;)
            {
                if (_kbhit())
                {
                    int k = _getch();
                    if (k == 0 || k == 224) return KEY_EXT | _getch();
                    return k;
                }

                int w = 0, h = 0;
                getSize(w, h);
                if (w != lastW || h != lastH)
                {
                    lastW = w;
                    lastH = h;
                    return KEY_RESIZE;
                }

                Sleep(30);
            }
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

        void getSize(int& w, int& h)
        {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (!GetConsoleScreenBufferInfo(g_out, &csbi)) { w = SCREEN_W; h = SCREEN_H; return; }
            w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }

        // Wipes the ENTIRE buffer, not just the visible window, so old child-process
        // output scrolled off-screen can't bleed back in when we redraw with that stupid lingering powershell.
        void fullClear(WORD attr)
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