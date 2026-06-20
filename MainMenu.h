#pragma once
#include "pch.h"
#include "FilePopulate.h"
#include "ConsoleUI.h"

namespace myutils
{
    class MainMenu
    {
    public:
        void run();

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

        FilePopulate          m_fp;
        std::vector<UtilFile> m_files;
        std::vector<Category> m_categories;

        void buildCategories();
        void drawChrome();                     
        std::vector<std::wstring> buildBanner() const;

        int  runView(const ViewSpec& v);

        void runCategory(const Category& cat);
        void launch(const UtilFile& f);
    };
}