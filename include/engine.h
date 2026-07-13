#ifndef OMITY_ENGINE_H
#define OMITY_ENGINE_H

#include "file.h"
#include <string>
#include <vector>
#include <atomic>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace Omity {

    class Engine {
    public:
        Engine();
        ~Engine();

        bool Initialize();                  
        void StartBackgroundLoop();         
        void Stop();                        
        void MinimizeMemoryUsage();         

    private:

        bool IsNetworkConnected();
        void ProcessFailedQueue();

        std::atomic<bool> m_isRunning;

        // 파일 감시 쓰레드
        std::thread m_fileWatcherThread;

        // 객체 선언
        File m_file;
        ZmqSender m_sender;

    #ifdef _WIN32
        HANDLE m_hDir;                     
    #endif

        std::vector<std::string> m_failedQueue;
    };

} // namespace Omity

#endif // OMITY_ENGINE_H
