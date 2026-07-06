#include "engine.h"

#ifdef _WIN32
	#include <windows.h>
	#include <psapi.h>
	#pragma comment(lib, "psapi.lib")
#else
	#include <sys/mman.h>
	#include <malloc.h>
#endif

namespace Omity {
	
	void Engine::MinimizeMemoryUsage() {
	#ifdef _WIN32
		HANDLE hProcess = GetCurrentProcess();
		SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);
	#else
		malloc_trim(0);
		madvise(nullptr, 0, MADV_DONTNEED);
	#endif
	}
}
