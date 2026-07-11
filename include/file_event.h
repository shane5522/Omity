#ifndef FILE_EVENT_H
#define FILE_EVENT_H

#include <string>

namespace Omity
{

    enum class FileAction
    {
        Added,
        Removed,
        Modified,
        RenamedOld,
        RenamedNew
    };

    // 파일 이벤트 데이터 구조체
    struct FileEvent
    {
        FileAction action;

        std::string filename;
        std::string path;

    };

}

#endif