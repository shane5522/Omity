#ifndef OMITY_ENGINE_H
#define OMITY_ENGINE_H

#include <string>
#include <vector>
#include <atomic>
#include "file.h"
#include "zmq_sender.h"
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
        bool StartPythonProcess();

    private:

        bool IsNetworkConnected();
        void ProcessFailedQueue();

        std::atomic<bool> m_isRunning;

        // 쓰레드 선언
        std::thread m_fileWatcherThread;

        // 프로세스 선언
        HANDLE m_pythonProcess;

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
