#include "file_event.h"
#include "json_serializer.h"
#include <string>

namespace Omity
{
	std::string Json::Serialize(const FileEvent& event)
	{
		return "{"
			"\"action\":\"" + ActionToString(event.action) + "\","
			"\"filename\":\"" + event.filename + "\","
			"\"path\":\"" + event.path + "\""
			"}";
	}
}