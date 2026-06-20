#include "pch.h"
#include "MainMenu.h"
#include "Config.h"

static void printUsage()
{
    std::wcout
        << L"MYUTILS - utility launcher\n\n"
        << L"  myutils                       scan saved folder, else current folder\n"
        << L"  myutils -dir \"C:\\path\"         scan a folder for this run only\n"
        << L"  myutils \"C:\\path\"              same (bare path)\n"
        << L"  myutils -dir set \"C:\\path\"     save a default folder, then scan it\n"
        << L"  myutils -dir clear            forget the saved folder\n"
        << L"  myutils -dir show             print the saved folder\n"
        << L"  myutils -h | --help           show this help\n";
}

int wmain(int argc, wchar_t* argv[])
{
    std::wstring oneOffDir;     
    bool         saveIt = false; 

    for (int i = 1; i < argc; ++i)
    {
        std::wstring a = argv[i];

        if (a == L"-h" || a == L"--help" || a == L"/?")
        {
            printUsage();
            return 0;
        }
        else if (a == L"-dir" || a == L"--dir" || a == L"-d")
        {
            std::wstring next = (i + 1 < argc) ? std::wstring(argv[i + 1]) : L"";

            if (next == L"set")
            {
                if (i + 2 < argc) { oneOffDir = argv[i + 2]; saveIt = true; i += 2; }
                else { std::wcout << L"Usage: myutils -dir set \"C:\\path\"\n"; return 1; }
            }
            else if (next == L"clear")
            {
                std::wcout << (myutils::config::clearScanDir()
                    ? L"Saved folder cleared.\n"
                    : L"No saved folder to clear.\n");
                return 0;
            }
            else if (next == L"show")
            {
                std::wstring saved = myutils::config::loadScanDir();
                std::wcout << (saved.empty() ? L"No saved folder.\n"
                    : L"Saved folder: " + saved + L"\n");
                return 0;
            }
            else if (!next.empty())
            {
                oneOffDir = next; ++i; 
            }
        }
        else if (oneOffDir.empty() && !a.empty() && a[0] != L'-')
        {
            oneOffDir = a; 
        }
    }

    std::wstring scanDir;
    if (!oneOffDir.empty())
    {
        if (saveIt) myutils::config::saveScanDir(oneOffDir);
        scanDir = oneOffDir;
    }
    else
    {
        scanDir = myutils::config::loadScanDir();
    }

    myutils::MainMenu menu;
    menu.run(scanDir);
    return 0;
}