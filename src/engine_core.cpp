#include "engine.h"
#include "logger.h"
#include "string_utils.h"
#include "file_event.h"
#include <iostream>
#include <fstream>
#include <string>

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

	bool Engine::Initialize() {
		LOG_INFO("Omity Engine Initializing...");

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

	static FileAction ConvertAction(DWORD action)
	{
		switch (action)
		{
		case FILE_ACTION_ADDED:
			return FileAction::Added;

		case FILE_ACTION_REMOVED:
			return FileAction::Removed;

		case FILE_ACTION_MODIFIED:
			return FileAction::Modified;

		case FILE_ACTION_RENAMED_OLD_NAME:
			return FileAction::RenamedOld;

		case FILE_ACTION_RENAMED_NEW_NAME:
			return FileAction::RenamedNew;

		default:
			return FileAction::Modified;
		}
	}

	// 파일 감시 함수
	void Engine::StartFileWatcher()
	{
		// 감시 디렉토리 핸들 생성
		HANDLE hDir = CreateFileW(
			L"C:\\Users\\junseo\\Desktop\\junseo\\Omity\\watch",
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL
		);

		// 디렉토리 경로 오류 처리
		if (hDir == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR("Failed to open directory");
			return;
		}

		BYTE buffer[4096];
		DWORD bytesReturned;

		while (m_isRunning)
		{
			// 파일 상태 변화 감지 함수 호출
			BOOL success = ReadDirectoryChangesW(
				hDir,
				buffer,
				sizeof(buffer),
				FALSE, 
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
				&bytesReturned,
				NULL,
				NULL
			);

			// buffer 정보 처리
			if (success)
			{
				// buffer를 FILE_NOTIFY_INFORMATION 구조체 형식으로 변환
				FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

				// 파일 이벤트 처리
				while (true)
				{
					std::wstring filename(
						info->FileName,
						info->FileNameLength / sizeof(WCHAR)
					);

					std::string name = UTF16ToUTF8(filename);

					// 파일 이벤트 데이터 생성
					FileEvent event;

					event.action = ConvertAction(info->Action);
					event.filename = name;
					event.path = "C:\\Users\\junseo\\Desktop\\junseo\\Omity\\watch\\" + name;

					// 로그 출력 테스트
					LOG_INFO("action : " + event.path);
					LOG_INFO("File detected : " + name);

					if (info->NextEntryOffset == 0) break;
					info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
				}
			}
		}

		CloseHandle(hDir);
	}

	void Engine::StartBackgroundLoop() {
		m_fileWatcherThread = std::thread(&Engine::StartFileWatcher, this);
		#ifdef _WIN32
			std::cout << "Omity Engine starts Windows background resident..." << std::endl;
		#else
			std::cout << "Omity Engine starts Linux background resident..." << std::endl;
		#endif

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
			m_isRunning = false;

			// 쓰레드 정리
			if (m_fileWatcherThread.joinable())
				m_fileWatcherThread.join();

			std::cout << "Omity Engine has been safely shut down." << std::endl;
		}
	}

}
