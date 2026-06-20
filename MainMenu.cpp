#include "pch.h"
#include "MainMenu.h"

namespace myutils
{
    using namespace ui;

    namespace {
        constexpr int ITEM_X = 4; 
        constexpr int ITEM_W = 72;  
        constexpr int ITEMS_Y = 11; 
        constexpr int VISIBLE = 8;   
        constexpr int HEAD_Y = 9;   
        constexpr int QUIT_Y = 20;
        constexpr int STATUS_Y = 22;  

        constexpr wchar_t TL = L'\u2554', TR = L'\u2557', BL = L'\u255A', BR = L'\u255D';
        constexpr wchar_t HZ = L'\u2550', VT = L'\u2551', LT = L'\u2560', RT = L'\u2563';

        const char* GLYPH[7][5] = {
            { "#   #", "## ##", "# # #", "#   #", "#   #" }, // M
            { "#   #", " # # ", "  #  ", "  #  ", "  #  " }, // Y
            { "#   #", "#   #", "#   #", "#   #", " ### " }, // U
            { "#####", "  #  ", "  #  ", "  #  ", "  #  " }, // T
            { "#####", "  #  ", "  #  ", "  #  ", "#####" }, // I
            { "#    ", "#    ", "#    ", "#    ", "#####" }, // L
            { "#####", "#    ", "#####", "    #", "#####" }, // S   God bless ASCII Glyph calculators
        };

        void borderLine(int y, wchar_t left, wchar_t mid, wchar_t right)
        {
            std::wstring s;
            s += left;
            s.append(SCREEN_W - 2, mid);
            s += right;
            putString(0, y, s, BORDER);
        }
    }

    std::vector<std::wstring> MainMenu::buildBanner() const
    {
        const int rows = 5, cols = 5, letters = 7;
        std::vector<std::wstring> out(rows);
        for (int r = 0; r < rows; ++r)
        {
            std::wstring line;
            for (int li = 0; li < letters; ++li)
            {
                if (li) line += L' ';
                const char* g = GLYPH[li][r];
                for (int c = 0; c < cols; ++c)
                    line += (g[c] == '#') ? L'\u2588' : L' ';
            }
            out[r] = line;
        }
        return out;
    }

    void MainMenu::buildCategories()
    {
        m_categories.clear();

        struct Def { FileType t; const wchar_t* label; };
        const Def defs[] = {
            // clean lineup for a clean utiltiy manager
            { FileType::Exe, L"Executables       (.exe)" },
            { FileType::Bat, L"Batch Scripts     (.bat)" },
            { FileType::Cmd, L"Command Scripts   (.cmd)" },
            { FileType::Ps1, L"PowerShell Scripts(.ps1)" },
        };

        for (const auto& d : defs)
        {
            Category cat;
            cat.type = d.t;
            cat.label = d.label;
            for (const auto& f : m_files)
                if (f.type == d.t)
                    cat.files.push_back(&f);

            if (!cat.files.empty())
                m_categories.push_back(std::move(cat));
        }
    }

    void MainMenu::drawChrome()
    {
        clear(BG);

        borderLine(0, TL, HZ, TR);
        borderLine(SCREEN_H - 1, BL, HZ, BR);
        borderLine(8, LT, HZ, RT);
        borderLine(21, LT, HZ, RT);

        for (int y = 1; y <= SCREEN_H - 2; ++y)
        {
            if (y == 8 || y == 21) continue; 
            putString(0, y, std::wstring(1, VT), BORDER);
            putString(SCREEN_W - 1, y, std::wstring(1, VT), BORDER);
        }

        auto banner = buildBanner();
        for (int i = 0; i < (int)banner.size(); ++i)
            putString(4, 2 + i, banner[i], TITLE);

        const std::wstring credit = L"by Ray Bunton 2026";
        putString(SCREEN_W - 2 - (int)credit.size(), 6, credit, ACCENT);
        putString(4, 7, L"- Utility Launcher  @_@ -", DIM);

        const std::wstring help =
            L"[\u2191\u2193] Move     [Enter/Space] Select     [Esc] Back / Quit";
        putString(4, 23, help, TEXT);
    }

    int MainMenu::runView(const ViewSpec& v)
    {
        const int n = (int)v.labels.size();
        const int total = n + 1;
        int sel = 0;
        int top = 0;

        for (int y = HEAD_Y; y <= QUIT_Y; ++y) clearRow(y);
        clearRow(STATUS_Y);
        putString(ITEM_X, HEAD_Y, fit(v.heading, ITEM_W), ACCENT);

        auto fixScroll = [&]()
            {
                if (sel < n)
                {
                    if (sel < top)               top = sel;
                    if (sel >= top + VISIBLE)     top = sel - VISIBLE + 1;
                }
                const int maxTop = (n > VISIBLE) ? (n - VISIBLE) : 0;
                if (top < 0)      top = 0;
                if (top > maxTop) top = maxTop;
            };

        auto draw = [&]()
            {
                for (int r = 0; r < VISIBLE; ++r) clearRow(ITEMS_Y + r);

                if (n == 0 && v.emptyAllowed)
                {
                    putBarCentered(ITEM_X, ITEMS_Y + 2, ITEM_W, v.placeholder, DIM);
                }
                else
                {
                    for (int r = 0; r < VISIBLE; ++r)
                    {
                        const int idx = top + r;
                        if (idx >= n) break;
                        const bool s = (sel == idx);
                        std::wstring lab = (s ? L"\u25BA " : L"  ")
                            + fit(v.labels[idx], ITEM_W - 4);
                        putBar(ITEM_X, ITEMS_Y + r, ITEM_W, lab, s ? SELECTED : TEXT, 1);
                    }
                    if (top > 0)
                        putString(ITEM_X + ITEM_W + 1, ITEMS_Y, L"\u25B2", ACCENT);
                    if (top + VISIBLE < n)
                        putString(ITEM_X + ITEM_W + 1, ITEMS_Y + VISIBLE - 1, L"\u25BC", ACCENT);
                }

                const bool pinSel = (sel == n);
                putBarCentered(2, QUIT_Y, SCREEN_W - 4, v.pinnedLabel, pinSel ? SELECTED : BARBG);

                clearRow(STATUS_Y);
                std::wstring st = pinSel ? v.pinnedStatus : (n > 0 ? v.statuses[sel] : L"");
                putString(ITEM_X, STATUS_Y, fit(st, ITEM_W), TEXT);
            };

        fixScroll();
        draw();

        for (;;)
        {
            int k = _getch();
            if (k == 0 || k == 224) 
            {
                int a = _getch();
                if (a == 72) { sel = (sel - 1 + total) % total;       fixScroll(); draw(); } 
                else if (a == 80) { sel = (sel + 1) % total;               fixScroll(); draw(); } 
                else if (a == 75) { return -1; }                                                
                else if (a == 77) { return sel; }                                                 
                else if (a == 71) { sel = 0;                               fixScroll(); draw(); } 
                else if (a == 79) { sel = total - 1;                       fixScroll(); draw(); } 
                else if (a == 73) { sel = std::max(0, sel - VISIBLE);      fixScroll(); draw(); } 
                else if (a == 81) { sel = std::min(total - 1, sel + VISIBLE); fixScroll(); draw(); } 
            }
            else
            {
                if (k == 13 || k == 32)     return sel;
                if (k == 27)                return -1;  
                if (k == 'q' || k == 'Q')   return -1;  
            }
        }
    }

    void MainMenu::runCategory(const Category& cat)
    {
        for (;;)
        {
            ViewSpec v;
            v.heading = L"MAIN MENU  \u00BB  " + cat.label;
            v.pinnedLabel = L"\u00AB  BACK";
            v.pinnedStatus = L"Return to the main menu";

            for (const UtilFile* f : cat.files)
            {
                std::wstring lab = f->name;
                if (!f->relDir.empty()) lab += L"   \u2014 " + f->relDir; 
                v.labels.push_back(lab);
                v.statuses.push_back(f->path);
            }

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size())
                return;

            launch(*cat.files[choice]);
            drawChrome();
        }
    }

    void MainMenu::launch(const UtilFile& f)
    {
        showCursor();
        clear(NORMAL);
        gotoxy(0, 0);

        writeRaw(L"  Launching: " + f.name + L"\r\n", NORMAL);
        writeRaw(L"  " + f.path + L"\r\n\r\n", NORMAL);
        writeRaw(L"  ----------------------------------------------------------------\r\n\r\n", NORMAL);

        std::wstring cmd;
        switch (f.type)
        {
        case FileType::Ps1:
            cmd = L"powershell -NoProfile -ExecutionPolicy Bypass -File \"" + f.path + L"\"";
            break;
        default: 
            cmd = L"\"" + f.path + L"\"";
            break;
        }

        std::wstring full = L"\"" + cmd + L"\"";
        _wsystem(full.c_str());

        writeRaw(L"\r\n\r\n  [ Finished - press any key to return to MYUTILS ]", NORMAL);
        _getch();

        hideCursor();
    }

    void MainMenu::run()
    {
        init();

        const std::wstring self = FilePopulate::exePath();
        const std::wstring root = FilePopulate::exeDirectory();

        m_files = m_fp.scan(root, self);
        buildCategories();

        drawChrome();

        for (;;)
        {
            // claude wrote this part if its cooked please lmk.
            ViewSpec v;
            v.heading = L"MAIN MENU";
            v.pinnedLabel = L"QUIT";
            v.pinnedStatus = L"Exit MYUTILS";
            v.emptyAllowed = true;
            v.placeholder = L"No utilities loaded";

            for (const auto& c : m_categories)
            {
                v.labels.push_back(c.label + L"   [" + std::to_wstring(c.files.size()) + L"]");
                v.statuses.push_back(std::to_wstring(c.files.size())
                    + L" file(s) found  -  press Enter to browse");
            }

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size())
                break; 

            runCategory(m_categories[choice]);
        }

        showCursor();
        clear(NORMAL);
        setColor(NORMAL);
        gotoxy(0, 0);
    }
}