#ifndef OMITY_FILE_H
#define OMITY_FILE_H

#include "file_event.h"
#include "json_serializer.h"
#include "zmq_sender.h"
#include <atomic>

namespace Omity
{
	class File {
	public:
		void StartFileWatcher(std::atomic<bool>& isRunning, ZmqSender& sender);
	private:
		Json json;
	};

}

#endif // OMITY_FILE_H