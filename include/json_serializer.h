#ifndef OMITY_JSON_SERIALIZER_H
#define OMITY_JSON_SERIALIZER_H

#include "file_event.h"
#include <string>

namespace Omity
{
	class Json {
	public:
		std::string EscapeJson(const std::string& str);
		std::string Serialize(const FileEvent& event);
	};
}

#endif // OMITY_JSON_SERIALIZER_H