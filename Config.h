#pragma once
#include "pch.h"

namespace myutils
{
    namespace config
    {
        std::wstring filePath();

        std::wstring              loadScanDir();
        bool                      saveScanDir(const std::wstring& dir);
        bool                      clearScanDir();

        std::vector<std::wstring> loadScanDirs();
        bool                      saveScanDirs(const std::vector<std::wstring>& dirs);

        std::wstring              loadPythonPath();
        bool                      savePythonPath(const std::wstring& path);

        std::vector<std::wstring> loadFolderOrder();
        bool                      saveFolderOrder(const std::vector<std::wstring>& order);

        std::map<std::wstring, int> loadFileOrder();
        bool                        saveFileOrder(const std::map<std::wstring, int>& order);

        std::vector<std::wstring>                         loadCustomFolders();
        std::map<std::wstring, std::vector<std::wstring>> loadCustomFolderFiles();
        bool                                              saveCustomFolders(
            const std::vector<std::wstring>& names,
            const std::map<std::wstring, std::vector<std::wstring>>& files);

        bool                      saveAll(
            const std::wstring& scanDir,
            const std::vector<std::wstring>& scanDirs,
            const std::wstring& pythonPath,
            const std::vector<std::wstring>& folderOrder,
            const std::map<std::wstring, int>& fileOrder,
            const std::vector<std::wstring>& customFolders,
            const std::map<std::wstring, std::vector<std::wstring>>& customFolderFiles);
    }
}