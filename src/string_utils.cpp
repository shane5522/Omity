#include "string_utils.h"
#include <windows.h>

namespace Omity
{

    std::string UTF16ToUTF8(const std::wstring& value)
    {
        if (value.empty())
            return "";

        int size = WideCharToMultiByte(
            CP_UTF8,
            0,
            value.data(),
            (int)value.size(),
            nullptr,
            0,
            nullptr,
            nullptr
        );

        std::string result(size, 0);

        WideCharToMultiByte(
            CP_UTF8,
            0,
            value.data(),
            (int)value.size(),
            result.data(),
            size,
            nullptr,
            nullptr
        );

        return result;
    }

}