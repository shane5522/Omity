#include "file_event.h"

namespace Omity
{
    std::string ActionToString(FileAction action)
    {
        switch (action)
        {
        case FileAction::Added:
            return "added";

        case FileAction::Removed:
            return "removed";

        case FileAction::Modified:
            return "modified";

        case FileAction::RenamedOld:
            return "renamed_old";

        case FileAction::RenamedNew:
            return "renamed_new";

        default:
            return "unknown";
        }
    }
}