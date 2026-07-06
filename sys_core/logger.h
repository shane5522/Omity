#ifndef OMITY_LOGGER_H
#define OMITY_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

namespace Omity {
	class Logger {
		public:

			static Logger& Instance();

			void Init(const std::string& filename);
			void Log(const std::string& level, const std::string& message);

		private:
			Logger() = default;
			~Logger();
			Logger(const Logger&) = delete;
			Logger& operator=(const Logger&) = delete;

			std::ofstream log_file_;
			std::mutex mutex_;
	};
}

#define LOG_INFO(msg) Omity::Logger::Instance().Log("INFO", msg)
#define LOG_WARN(msg) Omity::Logger::Instance().Log("WARN", msg)
#define LOG_ERROR(msg) Omity::Logger::Instance().Log("ERROR", msg)

#endif
