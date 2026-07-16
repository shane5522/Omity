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

    bool ZmqSender::StartPythonProcess()
    {
        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};

        si.cb = sizeof(si);

        std::wstring command = L"python .\\python\\ZeroMQ.py";

        BOOL success = CreateProcessW(
            nullptr,
            command.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (!success)
        {
            DWORD error = GetLastError();
            std::cout << "Python 실행 실패. Error : " << error << std::endl;
            return false;
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        return true;
    }

	void ZmqSender::Send(const std::string& data)
	{
		zmq::message_t message(data.begin(), data.end());

		m_socket.send(message, zmq::send_flags::none);

		LOG_INFO("[ZMQ SEND]" + data);

        bool success = StartPythonProcess();

        if (success)
            LOG_INFO("success");
	}
}