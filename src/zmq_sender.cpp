#include "zmq_sender.h"
#include "logger.h"
#include <windows.h>
#include <string>
#include <iostream>

namespace Omity
{
	ZmqSender::ZmqSender()
		: m_context(1),
		m_socket(m_context, zmq::socket_type::push)
	{
		m_socket.connect("tcp://127.0.0.1:5555");

		LOG_INFO("ZMQ Connected");
	}

	ZmqSender::~ZmqSender()
	{
		m_socket.close();
		m_context.close();
	}

	void ZmqSender::Send(const std::string& data)
	{
		zmq::message_t message(data.begin(), data.end());

		m_socket.send(message, zmq::send_flags::none);

		LOG_INFO("[ZMQ SEND]" + data);
	}

	void ZmqSender::Shutdown()
	{
		Send("shutdown");
	}
}