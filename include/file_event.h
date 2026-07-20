#ifndef OMITY_FILE_EVENT_H
#define OMITY_FILE_EVENT_H

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

    std::string ActionToString(FileAction action);
}

#endif