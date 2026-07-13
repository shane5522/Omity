#ifndef OMITY_ZMQ_SENDER_H
#define OMITY_ZMQ_SENDER_H

#include <string>

namespace Omity
{
	class ZmqSender
	{
	public:
		void Send(const std::string& data);

	private:

	};
}

#endif // OMITY_ZMQ_SENDER_H