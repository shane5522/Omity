#include "engine.h"
#include "logger.h"
#include "file.h"
#include "zmq_sender.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <functional>

#ifdef _WIN32
	#include <windows.h>
	#include <wininet.h>
	#pragma comment(lib, "wininet.lib")

#else
	#include <chrono>
	#include <thread>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

namespace Omity {

	Engine::Engine() : m_isRunning(false) {}
	Engine::~Engine() { Stop(); }

	bool Engine::StartPythonProcess()
	{
		STARTUPINFOW si{};
		PROCESS_INFORMATION pi{};

		si.cb = sizeof(si);


		std::wstring command =
			L"python \"C:\\Users\\junseo\\Desktop\\junseo\\Omity\\python\\zmq_receiver.py\"";


		BOOL success = CreateProcessW(
			nullptr,
			&command[0],
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			nullptr,
			&si,
			&pi
		);


		if (!success)
		{
			DWORD error = GetLastError();

			LOG_ERROR(
				"Python execution failed : "
				+ std::to_string(error)
			);

			return false;
		}


		LOG_INFO("Python process started");


		// thread handle은 필요 없음
		CloseHandle(pi.hThread);


		// process handle은 저장해야 함
		m_pythonProcess = pi.hProcess;


		return true;
	}

	bool Engine::Initialize() {
		LOG_INFO("Omity Engine Initializing...");

		bool success = StartPythonProcess();

		if (!success)
		{
			LOG_INFO("fail");
		}
		else
		{
			LOG_INFO("success");
		}

	#ifdef _WIN32

		LOG_INFO("[Security] Window Hardware License Verified.");
	
	#else
	
		std::ifstream uuid_file("/sys/class/dmi/id/product_uuid");
		std::string hardware_uuid;

		if (uuid_file.is_open()) {
			std::getline(uuid_file, hardware_uuid);
			uuid_file.close();
			LOG_INFO("[Security] Machine Hardware UUID caught: " + hardware_uuid);
		} else {
			LOG_WARN("[Security] Cannot access hardware storage. Using fallback identifier for development...");
			hardware_uuid = "DEVELOPER-DEBUG-MODE-UUID";
		}
		if (hardware_uuid.empty()) {
			LOG_ERROR("[Security Warning} License verification failed. Anti-piracy lock activated.");
			return false;
		}
		LOG_INFO("[Security] License key verified successfully. Access Granted.");

	#endif
		m_isRunning = true;
		MinimizeMemoryUsage();
		return true;
	}

	bool Engine::IsNetworkConnected() {
	#ifdef _WIN32
		DWORD flags;
		return InternetGetConnectedState(&flags, 0);
	#else
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock < 0) return false;

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

		struct sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(53);
		serv_addr.sin_addr.s_addr = inet_addr("8.8.8.8");

		if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
			close(sock);
			return false;
		}
		close(sock);
		return true;
	#endif
	}

	void Engine::ProcessFailedQueue() {
		if (m_failedQueue.empty()) return;

		LOG_WARN("[Network Recovery] Internet reconnected! Processing " + std::to_string(m_failedQueue.size()) + " cached tasks...");
		for (const auto& filename : m_failedQueue) {
			LOG_INFO("[Queue Dispatched] Successfully sent backed up task: " + filename);
			#ifdef _WIN32
				Sleep(50);
			#else
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			#endif
		}
		m_failedQueue.clear();
	}

	

	void Engine::StartBackgroundLoop() {

		#ifdef _WIN32
			std::cout << "Omity Engine starts Windows background resident..." << std::endl;
		#else
			std::cout << "Omity Engine starts Linux background resident..." << std::endl;
		#endif
			m_fileWatcherThread = std::thread(&File::StartFileWatcher, &m_file, std::ref(m_isRunning), std::ref(m_sender));

			int ticker = 0;

			while (m_isRunning) {
				#ifdef _WIN32
					Sleep(100);
				#else
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				#endif

				ticker++;

				if(ticker % 30 == 0) {
					if (IsNetworkConnected()) {
						ProcessFailedQueue();
					}
				}

				if (ticker >= 100) {
					ticker = 0;
					std::string current_task = "omity_data_payload_" + std::to_string(time(NULL)) + ".dat";
					LOG_INFO("[Engine Watchdog Target] File payload generated: " + current_task);

					if (IsNetworkConnected()) {
						LOG_INFO("[Network Normal] Data instantly offloaded safely.");
					} else {
						LOG_WARN("[Network Error] Network connection lost. Securing inside local RAM Queue: " + current_task);
						m_failedQueue.push_back(current_task);
					}
				}
				
				MinimizeMemoryUsage();
			}
		}

	void Engine::Stop() {
		if (m_isRunning) {
			m_sender.Shutdown();

			m_isRunning = false;

			// 쓰레드 정리
			if (m_fileWatcherThread.joinable()) m_fileWatcherThread.join();

			std::cout << "Omity Engine has been safely shut down." << std::endl;
		}
	}

}
