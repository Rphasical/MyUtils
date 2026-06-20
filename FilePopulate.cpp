#include "pch.h"
#include "FilePopulate.h"

namespace fs = std::filesystem;

namespace myutils
{
    std::wstring FilePopulate::exePath()
    {
        wchar_t buf[MAX_PATH] = {};

        DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);

        if (n == 0) return L"";
        return std::wstring(buf, n);
    }

    std::wstring FilePopulate::exeDirectory()
    {
        std::wstring p = exePath();

        if (p.empty()) 
            return fs::current_path().wstring();

        return fs::path(p).parent_path().wstring();
    }

    // Classification of our filetypes, mr. claude said adding .cmd would be beneficial so here we are, dont liek him touching my backend stuff
    bool FilePopulate::classify(const fs::path& p, FileType& out)
    {
        std::wstring ext = p.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](wchar_t c) { return (wchar_t)std::towlower(c); });

        if (ext == L".exe") 
        { out = FileType::Exe; return true; }

        if (ext == L".bat") 
        { out = FileType::Bat; return true; }

        if (ext == L".cmd") 
        { out = FileType::Cmd; return true; }

        if (ext == L".ps1") 
        { out = FileType::Ps1; return true; }

        return false;
    }

    std::vector<UtilFile> FilePopulate::scan(const std::wstring& rootDir, const std::wstring& selfExe)
    {
        std::vector<UtilFile> out;
        std::error_code ec;
        fs::path root(rootDir);

        std::wstring self = selfExe;
        std::transform(self.begin(), self.end(), self.begin(),
            [](wchar_t c) { return (wchar_t)std::towlower(c); });

        const auto opts = fs::directory_options::skip_permission_denied;
        fs::recursive_directory_iterator it(root, opts, ec);
        const fs::recursive_directory_iterator end;
        if (ec) return out; // We want to make sure no root is missing, so we kick into no executables if so

        for (; it != end; it.increment(ec))
        {
            if (ec) { ec.clear(); continue; } // skip anything we can't step into because whats the point

            std::error_code fec;
            if (!it->is_regular_file(fec) || fec) continue;

            FileType type;
            if (!classify(it->path(), type)) continue;

            std::wstring full = it->path().wstring();
            std::wstring fullLower = full;
            std::transform(fullLower.begin(), fullLower.end(), fullLower.begin(),
                [](wchar_t c) { return (wchar_t)std::towlower(c); });
            if (!self.empty() && fullLower == self) continue; // never list ourselves, would be funny though.

            UtilFile uf;
            uf.name = it->path().filename().wstring(); // basic utilfile widestring for reference
            uf.path = full;
            uf.type = type;

            fs::path rel = fs::relative(it->path().parent_path(), root, fec);
            uf.relDir = (fec || rel.empty() || rel == fs::path(L".")) ? L"" : rel.wstring();

            out.push_back(std::move(uf));
        }
        return out;
    }
}