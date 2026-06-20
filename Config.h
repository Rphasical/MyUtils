#pragma once
#include "pch.h"

namespace myutils
{
    namespace config
    {
        std::wstring filePath();                     
        std::wstring loadScanDir();                       
        bool         saveScanDir(const std::wstring& dir);  
        bool         clearScanDir();                        
    }
}