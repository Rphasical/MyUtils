#include "pch.h"
#include "Config.h"

namespace fs = std::filesystem;

namespace myutils
{
    namespace config
    {
        namespace {

            std::wstring appDataDir()
            {
                wchar_t buf[MAX_PATH] = {};
                DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH);
                if (n == 0 || n >= MAX_PATH) return L"";
                return std::wstring(buf, n);
            }

            std::string toUtf8(const std::wstring& w)
            {
                if (w.empty()) return std::string();
                int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
                    nullptr, 0, nullptr, nullptr);
                std::string s(len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
                    &s[0], len, nullptr, nullptr);
                return s;
            }

            std::wstring fromUtf8(const std::string& s)
            {
                if (s.empty()) return std::wstring();
                int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
                std::wstring w(len, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], len);
                return w;
            }

            std::string jsonEscape(const std::string& s)
            {
                std::string o;
                o.reserve(s.size() + 8);
                for (char c : s)
                {
                    switch (c)
                    {
                    case '\\': o += "\\\\"; break;
                    case '\"': o += "\\\""; break;
                    case '\n': o += "\\n";  break;
                    case '\r': o += "\\r";  break;
                    case '\t': o += "\\t";  break;
                    default:   o += c;      break;
                    }
                }
                return o;
            }

            std::string extractScanDir(const std::string& text)
            {
                const std::string key = "\"scanDir\"";
                size_t k = text.find(key);
                if (k == std::string::npos) return "";
                size_t colon = text.find(':', k + key.size());
                if (colon == std::string::npos) return "";
                size_t q = text.find('"', colon + 1);
                if (q == std::string::npos) return "";
                ++q; 

                std::string out;
                for (size_t i = q; i < text.size(); ++i)
                {
                    char c = text[i];
                    if (c == '\\' && i + 1 < text.size())
                    {
                        char n = text[++i];
                        switch (n)
                        {
                        case 'n':  out += '\n'; break;
                        case 'r':  out += '\r'; break;
                        case 't':  out += '\t'; break;
                        case '"':  out += '"';  break;
                        case '\\': out += '\\'; break;
                        case '/':  out += '/';  break;
                        default:   out += n;    break;
                        }
                    }
                    else if (c == '"')
                    {
                        break;
                    }
                    else
                    {
                        out += c;
                    }
                }
                return out;
            }

        }

        std::wstring filePath()
        {
            std::wstring base = appDataDir();
            if (base.empty()) return L"";
            return (fs::path(base) / L"MyUtils" / L"config.json").wstring();
        }

        std::wstring loadScanDir()
        {
            std::wstring path = filePath();
            if (path.empty()) return L"";

            std::ifstream in(fs::path(path), std::ios::binary);
            if (!in) return L"";

            std::string text((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
            return fromUtf8(extractScanDir(text));
        }

        bool saveScanDir(const std::wstring& dir)
        {
            std::wstring path = filePath();
            if (path.empty()) return false;

            std::error_code ec;
            fs::create_directories(fs::path(path).parent_path(), ec);

            std::ofstream out(fs::path(path), std::ios::binary | std::ios::trunc);
            if (!out) return false;

            out << "{\n  \"scanDir\": \"" << jsonEscape(toUtf8(dir)) << "\"\n}\n";
            return (bool)out;
        }

        bool clearScanDir()
        {
            std::wstring path = filePath();
            if (path.empty()) return false;

            std::error_code ec;
            bool removed = fs::remove(fs::path(path), ec);
            return removed && !ec;
        }
    }
}