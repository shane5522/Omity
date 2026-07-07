#include "engine.h"
#include "logger.h"
#include <string>
#include <iostream>
#ifdef _WIN32
	#include <windows.h>
	#include <tchar.h>
	#define _WIN32_WINNT 0x0505
#else
	#include <unistd.h>
	#include <sys/wait.h>
	#include <sys/types.h>
#endif

int main(int argc, char* argc[]) {

	Omity::Logger::Instance().Init("omity.log");

#ifdef _WIN32

	HWND hWnd = GetConsoleWindow();
	if (hWnd != NULL) {
		ShowWindow(hWnd, SW_HIDE);
	}

	if (argc > 1 && std::string(argv[1]) == "--child") {
		Omity::Engine engine;
		if (!engine.Initialize()) {
			std::cerr << "Omity engine initialization failed." << std::endl;
			return 1;
		}

		engine.StartBackgroundLoop();

		std::cout << "program has been completely shut down." << std::endl;
		return 0;
	}

	std::cout << "[Omity System] Windows Watchdog system started." << std::endl;

	while (true) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		std::string cmd = std::string(argv[0]) + " --child";
		char* lpCommandLine = const_cast<char*>(cmd.c_str());

		if (!CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			std::cerr << "Process creation failed! Error: " << GetLastError() << std::endl;
			Sleep(2000);
			continue;
		}

		std::cout << "[Watchdog] Monitoring engine process (PID: " << pi.dwProcessId << ")." << std::endl;

		WaitForSingleObject(pi.hProcess, INFINITE);
		
		std::cout << "\n [warning] Omity Windows engine process terminated unexpectedly!" << std::endl;
		std::cout << " [Watchdog] Restarting engine process immediately..\n" << std::endl;

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		Sleep(100);
	}
#else
	std::cout << "[Omity System] Ubuntu Linux Watchdog system started." << std::endl;

	while (true) {
		pid_t pid = fork();

		if (pid < 0) {
			std::cerr << "Process fork failed!" << std::endl;
			return 1;
		}
		if (pid == 0) {

			Omity::Engine engine;

			if (!engine.Initialize()) {
				std::cerr << " Omity engine initialization failed." << std::endl;
				return 1;
			}

			engine.StartBackgroundLoop();

			std::cout << "program has been completely shut down." << std::endl;
			return 0;
		}
		else {
			int status;
			std::cout << "[Watchdog] Monitoring engine process (PID: " << pid << ")." << std::endl;

			waitpid(pid, &status, 0);
			
			if (WIFSIGNALED(status) || WIFEXITED(status)) {
				std::cout << "\n [Warning] Omity Linux engine process terminated unexpectedly!" << std::endl;
				std::cout << " [Watchdog] Restarting engine process immediately...\n" << std::endl;
				usleep(100000);
			}
		}
	}
#endif
	return 0;
}
