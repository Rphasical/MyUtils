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

            std::string jsonUnescape(const std::string& s)
            {
                std::string out;
                for (size_t i = 0; i < s.size(); ++i)
                {
                    if (s[i] == '\\' && i + 1 < s.size())
                    {
                        char n = s[++i];
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
                    else
                    {
                        out += s[i];
                    }
                }
                return out;
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

                std::string raw;
                for (size_t i = q; i < text.size(); ++i)
                {
                    char c = text[i];
                    if (c == '\\' && i + 1 < text.size())
                    {
                        char n = text[++i];
                        switch (n)
                        {
                        case 'n':  raw += '\n'; break;
                        case 'r':  raw += '\r'; break;
                        case 't':  raw += '\t'; break;
                        case '"':  raw += '"';  break;
                        case '\\': raw += '\\'; break;
                        case '/':  raw += '/';  break;
                        default:   raw += n;    break;
                        }
                    }
                    else if (c == '"') { break; }
                    else { raw += c; }
                }
                return raw;
            }

            std::vector<std::string> extractStringArray(const std::string& text, const std::string& key)
            {
                std::vector<std::string> out;
                size_t k = text.find("\"" + key + "\"");
                if (k == std::string::npos) return out;
                size_t bracket = text.find('[', k);
                if (bracket == std::string::npos) return out;
                size_t end = text.find(']', bracket);
                if (end == std::string::npos) return out;

                size_t i = bracket + 1;
                while (i < end)
                {
                    size_t q = text.find('"', i);
                    if (q == std::string::npos || q >= end) break;
                    ++q;
                    std::string val;
                    for (; q < text.size() && q < end + 100; ++q)
                    {
                        if (text[q] == '\\' && q + 1 < text.size())
                        {
                            val += jsonUnescape(std::string(1, text[q]) + text[q + 1]);
                            ++q;
                        }
                        else if (text[q] == '"') { break; }
                        else { val += text[q]; }
                    }
                    out.push_back(val);
                    i = q + 1;
                }
                return out;
            }

            std::string extractStringValue(const std::string& text, const std::string& key)
            {
                size_t k = text.find("\"" + key + "\"");
                if (k == std::string::npos) return "";
                size_t colon = text.find(':', k + key.size() + 2);
                if (colon == std::string::npos) return "";
                size_t q = text.find('"', colon + 1);
                if (q == std::string::npos) return "";
                ++q;
                std::string val;
                for (size_t i = q; i < text.size(); ++i)
                {
                    if (text[i] == '\\' && i + 1 < text.size()) { val += jsonUnescape(std::string(1, text[i]) + text[i + 1]); ++i; }
                    else if (text[i] == '"') break;
                    else val += text[i];
                }
                return val;
            }

            std::map<std::string, int> extractStringIntMap(const std::string& text, const std::string& key)
            {
                std::map<std::string, int> out;
                size_t k = text.find("\"" + key + "\"");
                if (k == std::string::npos) return out;
                size_t brace = text.find('{', k);
                if (brace == std::string::npos) return out;
                size_t end = text.find('}', brace);
                if (end == std::string::npos) return out;

                size_t i = brace + 1;
                while (i < end)
                {
                    size_t qk = text.find('"', i);
                    if (qk == std::string::npos || qk >= end) break;
                    ++qk;
                    std::string mapKey;
                    for (; qk < text.size() && text[qk] != '"'; ++qk) mapKey += text[qk];
                    size_t colon = text.find(':', qk + 1);
                    if (colon == std::string::npos || colon >= end) break;
                    size_t numStart = colon + 1;
                    while (numStart < end && (text[numStart] == ' ' || text[numStart] == '\t' || text[numStart] == '\n' || text[numStart] == '\r')) ++numStart;
                    std::string numStr;
                    while (numStart < end && (std::isdigit(text[numStart]) || text[numStart] == '-')) numStr += text[numStart++];
                    if (!numStr.empty()) out[mapKey] = std::stoi(numStr);
                    i = numStart;
                }
                return out;
            }

            std::map<std::string, std::vector<std::string>> extractStringArrayMap(const std::string& text, const std::string& key)
            {
                std::map<std::string, std::vector<std::string>> out;
                size_t k = text.find("\"" + key + "\"");
                if (k == std::string::npos) return out;
                size_t i = text.find('{', k);
                if (i == std::string::npos) return out;
                ++i;

                auto readString = [&](size_t& p) -> std::string
                    {
                        std::string s;
                        for (; p < text.size(); ++p)
                        {
                            if (text[p] == '\\' && p + 1 < text.size()) { s += jsonUnescape(std::string(1, text[p]) + text[p + 1]); ++p; }
                            else if (text[p] == '"') { ++p; break; }
                            else s += text[p];
                        }
                        return s;
                    };

                while (i < text.size())
                {
                    while (i < text.size() && text[i] != '"' && text[i] != '}') ++i;
                    if (i >= text.size() || text[i] == '}') break;
                    ++i;
                    std::string mapKey = readString(i);

                    size_t bracket = text.find('[', i);
                    if (bracket == std::string::npos) break;
                    i = bracket + 1;

                    std::vector<std::string> vals;
                    while (i < text.size())
                    {
                        while (i < text.size() && text[i] != '"' && text[i] != ']') ++i;
                        if (i >= text.size() || text[i] == ']') { ++i; break; }
                        ++i;
                        vals.push_back(readString(i));
                    }
                    out[mapKey] = vals;
                }
                return out;
            }

            std::string readFile(const std::wstring& path)
            {
                std::ifstream in(fs::path(path), std::ios::binary);
                if (!in) return "";
                return std::string((std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());
            }

            std::string buildJson(
                const std::string& scanDir,
                const std::vector<std::string>& scanDirs,
                const std::string& pythonPath,
                const std::vector<std::string>& folderOrder,
                const std::map<std::string, int>& fileOrder,
                const std::vector<std::string>& customFolders,
                const std::map<std::string, std::vector<std::string>>& customFolderFiles)
            {
                std::string j = "{\n";

                j += "  \"scanDir\": \"" + jsonEscape(scanDir) + "\",\n";

                j += "  \"scanDirs\": [";
                for (size_t i = 0; i < scanDirs.size(); ++i)
                {
                    if (i) j += ", ";
                    j += "\"" + jsonEscape(scanDirs[i]) + "\"";
                }
                j += "],\n";

                j += "  \"pythonPath\": \"" + jsonEscape(pythonPath) + "\",\n";

                j += "  \"folderOrder\": [";
                for (size_t i = 0; i < folderOrder.size(); ++i)
                {
                    if (i) j += ", ";
                    j += "\"" + jsonEscape(folderOrder[i]) + "\"";
                }
                j += "],\n";

                j += "  \"customFolders\": [";
                for (size_t i = 0; i < customFolders.size(); ++i)
                {
                    if (i) j += ", ";
                    j += "\"" + jsonEscape(customFolders[i]) + "\"";
                }
                j += "],\n";

                j += "  \"customFolderFiles\": {";
                bool cfFirst = true;
                for (const auto& kv : customFolderFiles)
                {
                    if (!cfFirst) j += ",";
                    j += "\n    \"" + jsonEscape(kv.first) + "\": [";
                    for (size_t i = 0; i < kv.second.size(); ++i)
                    {
                        if (i) j += ", ";
                        j += "\"" + jsonEscape(kv.second[i]) + "\"";
                    }
                    j += "]";
                    cfFirst = false;
                }
                if (!customFolderFiles.empty()) j += "\n  ";
                j += "},\n";

                j += "  \"fileOrder\": {";
                bool first = true;
                for (const auto& kv : fileOrder)
                {
                    if (!first) j += ", ";
                    j += "\n    \"" + jsonEscape(kv.first) + "\": " + std::to_string(kv.second);
                    first = false;
                }
                if (!fileOrder.empty()) j += "\n  ";
                j += "}\n}\n";

                return j;
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
            std::string text = readFile(path);
            return fromUtf8(extractScanDir(text));
        }

        bool saveScanDir(const std::wstring& dir)
        {
            auto dirs = loadScanDirs();
            auto pyPath = loadPythonPath();
            auto fOrder = loadFolderOrder();
            auto fiOrder = loadFileOrder();
            auto cNames = loadCustomFolders();
            auto cFiles = loadCustomFolderFiles();
            return saveAll(dir, dirs, pyPath, fOrder, fiOrder, cNames, cFiles);
        }

        bool clearScanDir()
        {
            return saveScanDir(L"");
        }

        std::vector<std::wstring> loadScanDirs()
        {
            std::wstring path = filePath();
            if (path.empty()) return {};
            std::string text = readFile(path);
            auto raw = extractStringArray(text, "scanDirs");
            std::vector<std::wstring> out;
            for (const auto& s : raw) out.push_back(fromUtf8(s));
            return out;
        }

        bool saveScanDirs(const std::vector<std::wstring>& dirs)
        {
            auto scanDir = loadScanDir();
            auto pyPath = loadPythonPath();
            auto fOrder = loadFolderOrder();
            auto fiOrder = loadFileOrder();
            auto cNames = loadCustomFolders();
            auto cFiles = loadCustomFolderFiles();
            return saveAll(scanDir, dirs, pyPath, fOrder, fiOrder, cNames, cFiles);
        }

        std::wstring loadPythonPath()
        {
            std::wstring path = filePath();
            if (path.empty()) return L"";
            std::string text = readFile(path);
            return fromUtf8(extractStringValue(text, "pythonPath"));
        }

        bool savePythonPath(const std::wstring& pyPath)
        {
            auto scanDir = loadScanDir();
            auto dirs = loadScanDirs();
            auto fOrder = loadFolderOrder();
            auto fiOrder = loadFileOrder();
            auto cNames = loadCustomFolders();
            auto cFiles = loadCustomFolderFiles();
            return saveAll(scanDir, dirs, pyPath, fOrder, fiOrder, cNames, cFiles);
        }

        std::vector<std::wstring> loadFolderOrder()
        {
            std::wstring path = filePath();
            if (path.empty()) return {};
            std::string text = readFile(path);
            auto raw = extractStringArray(text, "folderOrder");
            std::vector<std::wstring> out;
            for (const auto& s : raw) out.push_back(fromUtf8(s));
            return out;
        }

        bool saveFolderOrder(const std::vector<std::wstring>& order)
        {
            auto scanDir = loadScanDir();
            auto dirs = loadScanDirs();
            auto pyPath = loadPythonPath();
            auto fiOrder = loadFileOrder();
            auto cNames = loadCustomFolders();
            auto cFiles = loadCustomFolderFiles();
            return saveAll(scanDir, dirs, pyPath, order, fiOrder, cNames, cFiles);
        }

        std::map<std::wstring, int> loadFileOrder()
        {
            std::wstring path = filePath();
            if (path.empty()) return {};
            std::string text = readFile(path);
            auto raw = extractStringIntMap(text, "fileOrder");
            std::map<std::wstring, int> out;
            for (const auto& kv : raw) out[fromUtf8(kv.first)] = kv.second;
            return out;
        }

        bool saveFileOrder(const std::map<std::wstring, int>& order)
        {
            auto scanDir = loadScanDir();
            auto dirs = loadScanDirs();
            auto pyPath = loadPythonPath();
            auto fOrder = loadFolderOrder();
            auto cNames = loadCustomFolders();
            auto cFiles = loadCustomFolderFiles();
            return saveAll(scanDir, dirs, pyPath, fOrder, order, cNames, cFiles);
        }

        std::vector<std::wstring> loadCustomFolders()
        {
            std::wstring path = filePath();
            if (path.empty()) return {};
            std::string text = readFile(path);
            auto raw = extractStringArray(text, "customFolders");
            std::vector<std::wstring> out;
            for (const auto& s : raw) out.push_back(fromUtf8(s));
            return out;
        }

        std::map<std::wstring, std::vector<std::wstring>> loadCustomFolderFiles()
        {
            std::wstring path = filePath();
            if (path.empty()) return {};
            std::string text = readFile(path);
            auto raw = extractStringArrayMap(text, "customFolderFiles");
            std::map<std::wstring, std::vector<std::wstring>> out;
            for (const auto& kv : raw)
            {
                std::vector<std::wstring> vals;
                for (const auto& s : kv.second) vals.push_back(fromUtf8(s));
                out[fromUtf8(kv.first)] = vals;
            }
            return out;
        }

        bool saveCustomFolders(
            const std::vector<std::wstring>& names,
            const std::map<std::wstring, std::vector<std::wstring>>& files)
        {
            auto scanDir = loadScanDir();
            auto dirs = loadScanDirs();
            auto pyPath = loadPythonPath();
            auto fOrder = loadFolderOrder();
            auto fiOrder = loadFileOrder();
            return saveAll(scanDir, dirs, pyPath, fOrder, fiOrder, names, files);
        }

        bool saveAll(
            const std::wstring& scanDir,
            const std::vector<std::wstring>& scanDirs,
            const std::wstring& pythonPath,
            const std::vector<std::wstring>& folderOrder,
            const std::map<std::wstring, int>& fileOrder,
            const std::vector<std::wstring>& customFolders,
            const std::map<std::wstring, std::vector<std::wstring>>& customFolderFiles)
        {
            std::wstring path = filePath();
            if (path.empty()) return false;

            std::error_code ec;
            fs::create_directories(fs::path(path).parent_path(), ec);

            std::vector<std::string> dirsUtf, fOrderUtf, cNamesUtf;
            std::map<std::string, int> fiOrderUtf;
            std::map<std::string, std::vector<std::string>> cFilesUtf;
            for (const auto& d : scanDirs)    dirsUtf.push_back(toUtf8(d));
            for (const auto& d : folderOrder) fOrderUtf.push_back(toUtf8(d));
            for (const auto& kv : fileOrder)  fiOrderUtf[toUtf8(kv.first)] = kv.second;
            for (const auto& n : customFolders) cNamesUtf.push_back(toUtf8(n));
            for (const auto& kv : customFolderFiles)
            {
                std::vector<std::string> vals;
                for (const auto& p : kv.second) vals.push_back(toUtf8(p));
                cFilesUtf[toUtf8(kv.first)] = vals;
            }

            std::string json = buildJson(
                toUtf8(scanDir), dirsUtf, toUtf8(pythonPath), fOrderUtf, fiOrderUtf,
                cNamesUtf, cFilesUtf);

            std::ofstream out(fs::path(path), std::ios::binary | std::ios::trunc);
            if (!out) return false;
            out << json;
            return (bool)out;
        }
    }
}