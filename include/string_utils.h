#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>

namespace Omity
{
    std::string UTF16ToUTF8(const std::wstring& value);
}

#endif