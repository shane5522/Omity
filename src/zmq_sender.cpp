#include "zmq_sender.h"
#include "logger.h"

namespace Omity
{
	void ZmqSender::Send(const std::string& data)
	{
		LOG_INFO("[ZMQ SEND]" + data);
	}
}