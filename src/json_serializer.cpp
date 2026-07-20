#include "file_event.h"
#include "json_serializer.h"
#include <string>

namespace Omity
{
    std::string Json::EscapeJson(const std::string& str)
    {
        std::string result;

        for (char c : str)
        {
            switch (c)
            {
            case '\\':
                result += "\\\\";
                break;

            case '"':
                result += "\\\"";
                break;

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\t':
                result += "\\t";
                break;

            default:
                result += c;
            }
        }
        return result;
    }

    std::string Json::Serialize(const FileEvent& event)
    {
        return "{"
            "\"action\":\"" + EscapeJson(ActionToString(event.action)) + "\","
            "\"filename\":\"" + EscapeJson(event.filename) + "\","
            "\"path\":\"" + EscapeJson(event.path) + "\""
            "}";
    }
}