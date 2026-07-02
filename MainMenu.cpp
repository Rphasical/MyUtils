#include "pch.h"
#include "MainMenu.h"
#include "Config.h"

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
            { "#####", "#    ", "#####", "    #", "#####" }, // S   Thank god for ASCII calculators
        };

        bool typeForPath(const std::wstring& path, FileType& out)
        {
            std::wstring ext = std::filesystem::path(path).extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](wchar_t c) { return (wchar_t)std::towlower(c); });

            if (ext == L".exe") { out = FileType::Exe; return true; }
            if (ext == L".bat") { out = FileType::Bat; return true; }
            if (ext == L".cmd") { out = FileType::Cmd; return true; }
            if (ext == L".ps1") { out = FileType::Ps1; return true; }
            if (ext == L".py") { out = FileType::Py;  return true; }
            return false;
        }

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

    std::wstring MainMenu::buildScanHeading() const
    {
        if (m_scanDirs.empty()) return L"MAIN MENU";
        std::wstring h = L"MAIN MENU      Scanning: " + m_scanDirs[0];
        if (m_scanDirs.size() > 1)
            h += L" +" + std::to_wstring(m_scanDirs.size() - 1) + L" more";
        return h;
    }

    void MainMenu::applyFileOrder()
    {
        for (auto& cat : m_categories)
        {
            std::stable_sort(cat.files.begin(), cat.files.end(),
                [&](const UtilFile* a, const UtilFile* b)
                {
                    auto ia = m_fileOrder.find(a->path);
                    auto ib = m_fileOrder.find(b->path);
                    bool ha = (ia != m_fileOrder.end());
                    bool hb = (ib != m_fileOrder.end());
                    if (ha && hb) return ia->second < ib->second;
                    if (ha) return true;
                    if (hb) return false;
                    return a->name < b->name;
                });
        }
    }

    void MainMenu::rescanAll()
    {
        const std::wstring self = FilePopulate::exePath();
        if (m_scanDirs.size() > 1)
            m_files = m_fp.scan(m_scanDirs, self);
        else if (!m_scanDirs.empty())
            m_files = m_fp.scan(m_scanDirs[0], self);
        else
            m_files.clear();
        buildCategories();
        applyFileOrder();
    }

    void MainMenu::buildCategories()
    {
        m_categories.clear();

        struct Def { FileType t; const wchar_t* label; };
        const Def defs[] = {
            { FileType::Exe, L"Executables       (.exe)" },
            { FileType::Bat, L"Batch Scripts     (.bat)" },
            { FileType::Cmd, L"Command Scripts   (.cmd)" },
            { FileType::Ps1, L"PowerShell Scripts(.ps1)" },
            { FileType::Py,  L"Python Scripts    (.py) " },
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

    // Blocks until the console window is at least 80x25. Windows Terminal can
    // ignore our resize request, so if the user has it too small we show a
    // message instead of letting the fixed-grid layout wrap and fold on itself.
    void MainMenu::ensureSize()
    {
        bool wasSmall = false;
        int lastW = -1, lastH = -1;

        for (;;)
        {
            int w = 0, h = 0;
            getSize(w, h);
            if (w >= SCREEN_W && h >= SCREEN_H)
            {
                if (wasSmall)
                {
                    applySize();
                    fullClear(BG);
                }
                return;
            }

            wasSmall = true;
            if (w != lastW || h != lastH)
            {
                lastW = w;
                lastH = h;
                fullClear(BG);
                gotoxy(0, 0);
                writeRaw(L"  MYUTILS needs a larger window.\r\n\r\n", TEXT);
                writeRaw(L"  Please resize the console to at least 80 x 25.\r\n", TEXT);
                writeRaw(L"  Current size: " + std::to_wstring(w) + L" x " + std::to_wstring(h) + L"\r\n\r\n", DIM);
                writeRaw(L"  The menu returns automatically once it fits (Esc quits).", DIM);
            }

            if (_kbhit())
            {
                int k = _getch();
                if (k == 0 || k == 224) _getch();
                else if (k == 27) { showCursor(); fullClear(NORMAL); setColor(NORMAL); exit(0); }
            }

            Sleep(50);
        }
    }

    void MainMenu::drawChrome()
    {
        ensureSize();
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
        putString(4, 7, L"- Utility Launcher -", DIM);

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
                    // Scroll hints added for nicer ui since the old one was shitty
                    if (top > 0)
                        putString(ITEM_X + ITEM_W + 1, ITEMS_Y, L"\u25B2", ACCENT);
                    if (top + VISIBLE < n)
                        putString(ITEM_X + ITEM_W + 1, ITEMS_Y + VISIBLE - 1, L"\u25BC", ACCENT);
                }

                // Pinned bottom bar (QUIT / BACK), highlights when selected so its appeasing to the eyeballs.
                const bool pinSel = (sel == n);
                putBarCentered(2, QUIT_Y, SCREEN_W - 4, v.pinnedLabel, pinSel ? SELECTED : BARBG);

                // Detail line!
                clearRow(STATUS_Y);
                std::wstring st = pinSel ? v.pinnedStatus : (n > 0 ? v.statuses[sel] : L"");
                putString(ITEM_X, STATUS_Y, fit(st, ITEM_W), TEXT);
            };

        fixScroll();
        draw();

        for (;;)
        {
            int k = readKey();
            if (k == KEY_RESIZE)
            {
                drawChrome();
                putString(ITEM_X, HEAD_Y, fit(v.heading, ITEM_W), ACCENT);
                fixScroll();
                draw();
            }
            else if (k & KEY_EXT)
            {
                int a = k & 0xFF;
                if (a == 72) { sel = (sel - 1 + total) % total;          fixScroll(); draw(); }
                else if (a == 80) { sel = (sel + 1) % total;                  fixScroll(); draw(); }
                else if (a == 75) { return -1; }
                else if (a == 77) { return sel; }
                else if (a == 71) { sel = 0;                                   fixScroll(); draw(); }
                else if (a == 79) { sel = total - 1;                           fixScroll(); draw(); }
                else if (a == 73) { sel = std::max(0, sel - VISIBLE);          fixScroll(); draw(); }
                else if (a == 81) { sel = std::min(total - 1, sel + VISIBLE);  fixScroll(); draw(); }
            }
            else
            {
                if (k == 13 || k == 32)   return sel;
                if (k == 27)              return -1;
                if (k == 'q' || k == 'Q') return -1;
            }
        }
    }

    void MainMenu::runReorder(std::vector<std::wstring>& items, const std::wstring& heading,
        std::function<void(const std::vector<std::wstring>&)> onSave)
    {
        int sel = 0;
        const std::wstring reorderHelp =
            L"[\u2191\u2193] Move sel   [+/-] Shift item   [Enter] Save   [Esc] Cancel";

        auto draw = [&]()
            {
                for (int y = HEAD_Y; y <= QUIT_Y; ++y) clearRow(y);
                clearRow(STATUS_Y);
                putString(ITEM_X, HEAD_Y, fit(heading, ITEM_W), ACCENT);
                putString(ITEM_X, STATUS_Y, fit(reorderHelp, ITEM_W), DIM);

                const int n = (int)items.size();
                const int top = std::max(0, std::min(sel - VISIBLE / 2, n - VISIBLE));
                for (int r = 0; r < VISIBLE; ++r)
                {
                    const int idx = top + r;
                    if (idx >= n) { clearRow(ITEMS_Y + r); continue; }
                    const bool s = (sel == idx);
                    std::wstring lab = (s ? L"\u25BA " : L"  ") + fit(items[idx], ITEM_W - 4);
                    putBar(ITEM_X, ITEMS_Y + r, ITEM_W, lab, s ? SELECTED : TEXT, 1);
                }
                putBarCentered(2, QUIT_Y, SCREEN_W - 4, L"SAVE & RETURN", BARBG);
            };

        draw();

        for (;;)
        {
            int k = readKey();
            const int n = (int)items.size();
            if (k == KEY_RESIZE)
            {
                drawChrome();
                draw();
            }
            else if (k & KEY_EXT)
            {
                int a = k & 0xFF;
                if (a == 72 && sel > 0) { --sel; draw(); }
                else if (a == 80 && sel < n - 1) { ++sel; draw(); }
            }
            else if ((k == '+' || k == '=') && sel > 0)
            {
                std::swap(items[sel], items[sel - 1]);
                --sel;
                draw();
            }
            else if (k == '-' && sel < (int)items.size() - 1)
            {
                std::swap(items[sel], items[sel + 1]);
                ++sel;
                draw();
            }
            else if (k == 13)
            {
                onSave(items);
                return;
            }
            else if (k == 27) { return; }
        }
    }

    void MainMenu::runManageFolders()
    {
        for (;;)
        {
            drawChrome();
            ViewSpec v;
            v.heading = L"SETTINGS  \u00BB  Manage Folders";
            v.pinnedLabel = L"\u00AB  BACK";
            v.pinnedStatus = L"Return to Settings";

            std::vector<std::wstring> dirs = m_scanDirs;
            for (const auto& d : dirs)
            {
                v.labels.push_back(d);
                v.statuses.push_back(L"Press Del to remove   |   Reorder via Settings");
            }
            v.labels.push_back(L"  [ + Add current working directory ]");
            v.statuses.push_back(L"Adds the folder you launched myutils from");
            v.labels.push_back(L"  [ Reorder folders ]");
            v.statuses.push_back(L"Change which folder appears first in the scan heading");

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size()) return;

            const int addIdx = (int)dirs.size();
            const int reordIdx = addIdx + 1;

            if (choice == addIdx)
            {
                std::error_code ec;
                auto cwd = std::filesystem::current_path(ec);
                if (!ec)
                {
                    std::wstring cwdStr = cwd.wstring();
                    if (std::find(dirs.begin(), dirs.end(), cwdStr) == dirs.end())
                    {
                        dirs.push_back(cwdStr);
                        m_scanDirs = dirs;
                        config::saveScanDirs(dirs);
                        rescanAll();
                    }
                }
            }
            else if (choice == reordIdx)
            {
                std::vector<std::wstring> ordered = dirs;
                runReorder(ordered, L"SETTINGS  \u00BB  Reorder Folders",
                    [&](const std::vector<std::wstring>& saved)
                    {
                        m_scanDirs = saved;
                        m_folderOrder = saved;
                        config::saveScanDirs(saved);
                        config::saveFolderOrder(saved);
                        rescanAll();
                    });
            }
            else if (choice < addIdx)
            {
                dirs.erase(dirs.begin() + choice);
                m_scanDirs = dirs;
                config::saveScanDirs(dirs);
                rescanAll();
            }
        }
    }

    void MainMenu::runManageFileOrder(const Category& cat)
    {
        std::vector<std::wstring> paths;
        for (const auto* f : cat.files) paths.push_back(f->path);

        runReorder(paths,
            L"SETTINGS  \u00BB  File Order: " + cat.label,
            [&](const std::vector<std::wstring>& saved)
            {
                for (int i = 0; i < (int)saved.size(); ++i)
                    m_fileOrder[saved[i]] = i;
                config::saveFileOrder(m_fileOrder);
                applyFileOrder();
            });
    }

    void MainMenu::saveCustom()
    {
        config::saveCustomFolders(m_customFolders, m_customFiles);
    }

    std::wstring MainMenu::promptText(const std::wstring& heading)
    {
        for (int y = HEAD_Y; y <= QUIT_Y; ++y) clearRow(y);
        clearRow(STATUS_Y);
        putString(ITEM_X, HEAD_Y, fit(heading, ITEM_W), ACCENT);
        putString(ITEM_X, STATUS_Y, L"[Enter] Confirm   [Esc] Cancel   [Backspace] Delete", DIM);

        std::wstring text;
        auto paint = [&]()
            {
                putBar(ITEM_X, ITEMS_Y + 1, ITEM_W, L"> " + fit(text, ITEM_W - 6) + L"_", TEXT, 1);
            };
        paint();

        for (;;)
        {
            wint_t k = _getwch();
            if (k == 0 || k == 0xE0) { _getwch(); continue; }
            if (k == 13) break;
            if (k == 27) { text.clear(); break; }
            if (k == 8)
            {
                if (!text.empty()) { text.pop_back(); paint(); }
                continue;
            }
            if (k >= 32 && k != L'"' && k != L'\\' && k != L'[' && k != L']' && (int)text.size() < 48)
            {
                text += (wchar_t)k;
                paint();
            }
        }

        while (!text.empty() && text.front() == L' ') text.erase(text.begin());
        while (!text.empty() && text.back() == L' ') text.pop_back();
        return text;
    }

    void MainMenu::runCustomFolder(const std::wstring& name)
    {
        for (;;)
        {
            ViewSpec v;
            v.heading = L"MAIN MENU  \u00BB  " + name;
            v.pinnedLabel = L"\u00AB  BACK";
            v.pinnedStatus = L"Return to the main menu";
            v.emptyAllowed = true;
            v.placeholder = L"Empty folder  -  add files via Settings \u00BB Custom Folders";

            const auto it = m_customFiles.find(name);
            const std::vector<std::wstring> paths =
                (it == m_customFiles.end()) ? std::vector<std::wstring>{} : it->second;

            for (const auto& p : paths)
            {
                std::error_code ec;
                const bool exists = std::filesystem::exists(p, ec) && !ec;
                std::wstring lab = std::filesystem::path(p).filename().wstring();
                if (!exists) lab += L"   (missing)";
                v.labels.push_back(lab);
                v.statuses.push_back(p);
            }

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size()) return;

            const std::wstring& p = paths[choice];
            std::error_code ec;
            if (!std::filesystem::exists(p, ec) || ec) continue;

            FileType t;
            if (!typeForPath(p, t)) continue;

            UtilFile uf;
            uf.name = std::filesystem::path(p).filename().wstring();
            uf.path = p;
            uf.type = t;
            launch(uf);
            drawChrome();
        }
    }

    void MainMenu::runAddFilesToCustom(const std::wstring& name)
    {
        for (;;)
        {
            drawChrome();
            ViewSpec v;
            v.heading = L"SETTINGS  \u00BB  Add to: " + name;
            v.pinnedLabel = L"\u00AB  DONE";
            v.pinnedStatus = L"Return to the folder";
            v.emptyAllowed = true;
            v.placeholder = L"Every scanned file is already in this folder";

            auto& paths = m_customFiles[name];
            std::vector<const UtilFile*> avail;
            for (const auto& f : m_files)
                if (std::find(paths.begin(), paths.end(), f.path) == paths.end())
                    avail.push_back(&f);

            for (const auto* f : avail)
            {
                std::wstring lab = f->name;
                if (!f->relDir.empty()) lab += L"   \u2014 " + f->relDir;
                v.labels.push_back(lab);
                v.statuses.push_back(L"Enter to add  |  " + f->path);
            }

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size()) return;

            paths.push_back(avail[choice]->path);
            saveCustom();
        }
    }

    void MainMenu::runEditCustomFolder(std::wstring name)
    {
        for (;;)
        {
            drawChrome();
            ViewSpec v;
            v.heading = L"SETTINGS  \u00BB  Custom Folder: " + name;
            v.pinnedLabel = L"\u00AB  BACK";
            v.pinnedStatus = L"Return to Custom Folders";

            std::vector<std::wstring> paths = m_customFiles[name];
            for (const auto& p : paths)
            {
                v.labels.push_back(std::filesystem::path(p).filename().wstring());
                v.statuses.push_back(L"Enter to remove  |  " + p);
            }
            v.labels.push_back(L"  [ + Add files from scan ]");
            v.statuses.push_back(L"Pick scanned files to put in this folder");
            v.labels.push_back(L"  [ Rename this folder ]");
            v.statuses.push_back(L"Change the label shown in the main menu");
            v.labels.push_back(L"  [ Delete this folder ]");
            v.statuses.push_back(L"Removes the folder only, never the files");

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size()) return;

            const int addIdx = (int)paths.size();
            const int renIdx = addIdx + 1;
            const int delIdx = addIdx + 2;

            if (choice == addIdx)
            {
                runAddFilesToCustom(name);
            }
            else if (choice == renIdx)
            {
                std::wstring fresh = promptText(L"SETTINGS  \u00BB  Rename Folder  -  type a new name");
                if (!fresh.empty() && fresh != name
                    && std::find(m_customFolders.begin(), m_customFolders.end(), fresh) == m_customFolders.end())
                {
                    auto it = std::find(m_customFolders.begin(), m_customFolders.end(), name);
                    if (it != m_customFolders.end()) *it = fresh;
                    m_customFiles[fresh] = m_customFiles[name];
                    m_customFiles.erase(name);
                    name = fresh;
                    saveCustom();
                }
            }
            else if (choice == delIdx)
            {
                auto it = std::find(m_customFolders.begin(), m_customFolders.end(), name);
                if (it != m_customFolders.end()) m_customFolders.erase(it);
                m_customFiles.erase(name);
                saveCustom();
                return;
            }
            else if (choice < addIdx)
            {
                auto& live = m_customFiles[name];
                auto pit = std::find(live.begin(), live.end(), paths[choice]);
                if (pit != live.end()) live.erase(pit);
                saveCustom();
            }
        }
    }

    void MainMenu::runManageCustomFolders()
    {
        for (;;)
        {
            drawChrome();
            ViewSpec v;
            v.heading = L"SETTINGS  \u00BB  Custom Folders";
            v.pinnedLabel = L"\u00AB  BACK";
            v.pinnedStatus = L"Return to Settings";

            for (const auto& name : m_customFolders)
            {
                const auto it = m_customFiles.find(name);
                const size_t cnt = (it == m_customFiles.end()) ? 0 : it->second.size();
                v.labels.push_back(name + L"   [" + std::to_wstring(cnt) + L"]");
                v.statuses.push_back(L"Enter to edit this folder");
            }
            v.labels.push_back(L"  [ + New custom folder ]");
            v.statuses.push_back(L"Create a new personally labelled folder");

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size()) return;

            const int newIdx = (int)m_customFolders.size();
            if (choice == newIdx)
            {
                std::wstring fresh = promptText(L"SETTINGS  \u00BB  New Custom Folder  -  type a name");
                if (!fresh.empty()
                    && std::find(m_customFolders.begin(), m_customFolders.end(), fresh) == m_customFolders.end())
                {
                    m_customFolders.push_back(fresh);
                    m_customFiles[fresh] = {};
                    saveCustom();
                    runEditCustomFolder(fresh);
                }
            }
            else
            {
                const std::wstring picked = m_customFolders[choice];
                runEditCustomFolder(picked);
            }
        }
    }

    void MainMenu::runSettings()
    {
        for (;;)
        {
            drawChrome();
            ViewSpec v;
            v.heading = L"SETTINGS";
            v.pinnedLabel = L"\u00AB  BACK";
            v.pinnedStatus = L"Return to main menu";

            v.labels.push_back(L"Manage Folders");
            v.statuses.push_back(L"Add, remove, or reorder scanned folders");

            v.labels.push_back(L"Custom Folders");
            v.statuses.push_back(L"Create and fill your own personally labelled folders");

            for (const auto& cat : m_categories)
            {
                v.labels.push_back(L"File Order: " + cat.label);
                v.statuses.push_back(L"Reorder files inside this category");
            }

            const int choice = runView(v);
            if (choice < 0 || choice == (int)v.labels.size()) return;

            if (choice == 0) { runManageFolders(); }
            else if (choice == 1) { runManageCustomFolders(); }
            else
            {
                const int catIdx = choice - 2;
                if (catIdx < (int)m_categories.size())
                    runManageFileOrder(m_categories[catIdx]);
            }
            drawChrome();
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
        fullClear(NORMAL);
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
        case FileType::Py:
        {
            std::wstring interp = m_pythonPath.empty() ? L"python" : m_pythonPath;
            cmd = interp + L" \"" + f.path + L"\"";
            break;
        }
        default:
            cmd = L"\"" + f.path + L"\"";
            break;
        }

        std::wstring full = L"\"" + cmd + L"\"";
        _wsystem(full.c_str());

        writeRaw(L"\r\n\r\n  [ Finished - press any key to return to MYUTILS ]", NORMAL);
        _getch();

        fullClear(BG);
        hideCursor();
    }

    void MainMenu::run(const std::wstring& scanDir)
    {
        init();

        const std::wstring self = FilePopulate::exePath();

        m_scanDirs = config::loadScanDirs();
        m_pythonPath = config::loadPythonPath();
        m_folderOrder = config::loadFolderOrder();
        m_fileOrder = config::loadFileOrder();
        m_customFolders = config::loadCustomFolders();
        m_customFiles = config::loadCustomFolderFiles();

        if (!scanDir.empty())
        {
            if (std::find(m_scanDirs.begin(), m_scanDirs.end(), scanDir) == m_scanDirs.end())
                m_scanDirs.insert(m_scanDirs.begin(), scanDir);
        }
        else if (m_scanDirs.empty())
        {
            std::error_code ec;
            std::filesystem::path cwd = std::filesystem::current_path(ec);
            m_scanDirs.push_back(ec ? FilePopulate::exeDirectory() : cwd.wstring());
        }

        rescanAll();
        drawChrome();

        for (;;)
        {
            ViewSpec v;
            v.heading = buildScanHeading();
            v.pinnedLabel = L"QUIT";
            v.pinnedStatus = L"Exit MYUTILS";
            v.emptyAllowed = true;
            v.placeholder = m_categories.empty()
                ? L"No utilities loaded  -  nothing runnable in configured folders"
                : L"No utilities loaded";

            for (const auto& c : m_categories)
            {
                v.labels.push_back(c.label + L"   [" + std::to_wstring(c.files.size()) + L"]");
                v.statuses.push_back(std::to_wstring(c.files.size())
                    + L" file(s) found  -  press Enter to browse");
            }

            for (const auto& name : m_customFolders)
            {
                const auto it = m_customFiles.find(name);
                const size_t cnt = (it == m_customFiles.end()) ? 0 : it->second.size();
                v.labels.push_back(L"\u2605 " + name + L"   [" + std::to_wstring(cnt) + L"]");
                v.statuses.push_back(L"Custom folder  -  press Enter to browse");
            }

            v.labels.push_back(L"Settings");
            v.statuses.push_back(L"Manage folders, custom folders, file order, and Python interpreter");

            const int choice = runView(v);
            const int catCount = (int)m_categories.size();
            const int settingsIdx = catCount + (int)m_customFolders.size();
            const int quitIdx = settingsIdx + 1;

            if (choice < 0 || choice == quitIdx) break;

            if (choice == settingsIdx) { runSettings(); drawChrome(); }
            else if (choice < catCount) runCategory(m_categories[choice]);
            else if (choice < settingsIdx) runCustomFolder(m_customFolders[choice - catCount]);
        }

        showCursor();
        clear(NORMAL);
        setColor(NORMAL);
        gotoxy(0, 0);
    }
}