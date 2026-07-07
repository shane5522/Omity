#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Omity {
	Logger& Logger::Instance() {
		static Logger instance;
		return instance;
	}

	Logger::~Logger() {
		if (log_file_.is_open()) {
			log_file_.close();
		}
	}

	void Logger::Init(const std::string& filename) {
		std::lock_guard<std::mutex> lock(mutex_);

		log_file_.open(filename, std::ios::out | std::ios::app);
	}

	void Logger::Log(const std::string& level, const std::string& message) {
		std::lock_guard<std::mutex> lock(mutex_);
		if(!log_file_.is_open()) return;

		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");

		std::string log_line = "[" + ss.str() + "] [" + level + "] " + message;

		std::cout << log_line << std::endl;

		log_file_ << log_line << std::endl;
	}
}
