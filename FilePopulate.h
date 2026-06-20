#pragma once
#include "pch.h"

namespace myutils
{
    enum class FileType { Exe, Bat, Cmd, Ps1 };

    struct UtilFile
    {

        std::wstring name;   
        std::wstring relDir;  
        std::wstring path;    

        FileType     type = FileType::Exe;

    };

    class FilePopulate
    {
    public:

        FilePopulate() = default;
        ~FilePopulate() = default;

        std::vector<UtilFile> scan(const std::wstring& rootDir, const std::wstring& selfExe = L"");

        static std::wstring exePath();      
        static std::wstring exeDirectory();  

    private:
        static bool classify(const std::filesystem::path& p, FileType& out);
    };
}