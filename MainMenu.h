#pragma once
#include "pch.h"
#include "FilePopulate.h"
#include "ConsoleUI.h"

namespace myutils
{
    class MainMenu
    {
    public:
        void run(const std::wstring& scanDir = L"");

    private:
        struct Category
        {
            FileType                     type;
            std::wstring                 label;
            std::vector<const UtilFile*> files;
        };

        struct ViewSpec
        {
            std::wstring              heading;
            std::vector<std::wstring> labels;
            std::vector<std::wstring> statuses;
            std::wstring              pinnedLabel;
            std::wstring              pinnedStatus;
            bool                      emptyAllowed = false;
            std::wstring              placeholder;
        };

        FilePopulate              m_fp;
        std::vector<UtilFile>     m_files;
        std::vector<Category>     m_categories;

        std::vector<std::wstring>    m_scanDirs;
        std::wstring                 m_pythonPath;
        std::vector<std::wstring>    m_folderOrder;
        std::map<std::wstring, int>  m_fileOrder;

        std::vector<std::wstring>                         m_customFolders;
        std::map<std::wstring, std::vector<std::wstring>> m_customFiles;

        void buildCategories();
        void drawChrome();
        std::vector<std::wstring> buildBanner() const;

        int  runView(const ViewSpec& v);
        void runCategory(const Category& cat);
        void launch(const UtilFile& f);

        void runCustomFolder(const std::wstring& name);
        void runManageCustomFolders();
        void runEditCustomFolder(std::wstring name);
        void runAddFilesToCustom(const std::wstring& name);
        void saveCustom();
        std::wstring promptText(const std::wstring& heading);

        void runSettings();
        void runManageFolders();
        void runManageFileOrder(const Category& cat);
        void runReorder(std::vector<std::wstring>& items, const std::wstring& heading,
            std::function<void(const std::vector<std::wstring>&)> onSave);

        std::wstring buildScanHeading() const;
        void         applyFileOrder();
        void         rescanAll();
        void         ensureSize();
    };
}