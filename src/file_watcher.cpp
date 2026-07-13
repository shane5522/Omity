#include "file.h"
#include "logger.h"
#include "file_event.h"
#include "string_utils.h"
#include "json_serializer.h"
#include "zmq_sender.h"

#include <atomic>
#include <string>

#ifdef _WIN32
	#include <windows.h>

#else
	#include <chrono>
	#include <thread>
#endif

namespace Omity
{
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
	void File::StartFileWatcher(std::atomic<bool>& isRunning, ZmqSender& sender)
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

		while (isRunning.load())
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

					// action을 string타입으로 변환
					std::string jsonData = json.Serialize(event);

					// zmq 전송
					sender.Send(jsonData);

					// 로그 출력 테스트
					LOG_INFO(jsonData);

					if (info->NextEntryOffset == 0) break;
					info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
				}
			}
		}

		CloseHandle(hDir);
	}
}