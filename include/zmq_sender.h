#ifndef OMITY_ZMQ_SENDER_H
#define OMITY_ZMQ_SENDER_H

#include <string>
#include <zmq.hpp>

namespace Omity
{
	class ZmqSender
	{
	public:
		ZmqSender();
		~ZmqSender();
		void Send(const std::string& data);
		bool StartPythonProcess();

	private:
		zmq::context_t m_context;
		zmq::socket_t m_socket;           
	};
}

#endif // OMITY_ZMQ_SENDER_H