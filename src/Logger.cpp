#include "Logger.hpp"
#include "Engine.hpp"
#include "Utils.hpp"

#include <stdlib.h>

namespace NovaEngine
{
	const char* Logger::DEFAULT_COLOR = "\033[39m\033[49m";
	const char* Logger::INFO_COLOR = "\033[34m";
	const char* Logger::WARN_COLOR = "\033[33m";
	const char* Logger::ERROR_COLOR = "\033[31m";

	std::unordered_map<std::string, Logger*> Logger::loggers_ = std::unordered_map<std::string, Logger*>();
	std::queue<Logger::LogInfo> Logger::logQueue_;
	std::mutex Logger::mutex_;
	std::condition_variable Logger::cv_;
	bool Logger::shouldTerminate_ = false;
	std::optional<std::thread> Logger::logHandlerThread_;

	void Logger::onAbortHandler(int n)
	{
		std::cout << "ON ABORT!" << std::endl;
		for (std::pair<std::string, Logger*> p : Logger::loggers_)
		{
			p.second->error("TERMINATED !!!");
			p.second->terminate();
		}
	}

	std::string& Logger::date()
	{
		static std::optional<std::string> dateString;
		if (!dateString.has_value())
		{
			std::stringstream ss;
			std::time_t t = std::time(0);
			std::tm* now = std::localtime(&t);
			ss << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday;
			dateString = ss.str();
		}
		return dateString.value();
	}

	Logger* Logger::get(const char* name)
	{
		static std::string logPath = Utils::Path::combine(NovaEngine::Engine::executablePath(), "logs");
		static size_t logPathLength = logPath.size();
		static bool isInitialized = false;

		if (!isInitialized)
		{
			if (!std::filesystem::exists(logPath))
				std::filesystem::create_directory(logPath);

			std::signal(SIGABRT, &onAbortHandler);

			isInitialized = true;
		}

		std::string fileName = std::string(name == nullptr ? "" : name) + Logger::date();

		if (loggers_.find(fileName) == loggers_.end())
		{
			std::filesystem::path p = logPath;

			size_t version = 0;
			for (const auto& entry : std::filesystem::directory_iterator(p))
			{
				std::string foundName = entry.path().filename();
				foundName.resize(foundName.size() - 4);
				foundName[foundName.size()] = '\0';

				if (version == 0 && foundName == fileName)
				{
					version = 1;
				}
				else if (strncmp(foundName.c_str(), fileName.c_str(), fileName.size()) == 0)
				{
					size_t v = static_cast<size_t>(atoi(&foundName[fileName.size() + 1])) + 1;
					std::cout << "v: " << v << std::endl;
					if (v > version)
						version = v;
				}
			}
			std::string generatedFileName = fileName;
			if (version != 0)
				generatedFileName += "-" + std::to_string(version);

			std::string path = Utils::Path::combine(logPath, generatedFileName + ".log");
			loggers_[fileName] = new Logger(path.c_str());
		}

		return loggers_[fileName];
	}

	void Logger::terminate()
	{
		shouldTerminate_ = true;

		if (logHandlerThread_.has_value())
		{
			cv_.notify_one();
			if (logHandlerThread_.value().joinable())
				logHandlerThread_.value().join();
		}

		for (const auto& pair : loggers_)
			delete pair.second;
	}

	Logger::Logger(const char* path) : logFile_(path)
	{
		if (!logHandlerThread_.has_value())
			logHandlerThread_.emplace(std::thread([&]() {
			time_t rawtime;
			struct tm* timeinfo;
			char timeBuf[10] = {};

			while (!shouldTerminate_)
			{
				std::unique_lock<std::mutex> lock(mutex_);
				cv_.wait(lock);
				lock.unlock();

				while (logQueue_.size() > 0)
				{
					Logger::LogInfo& info = logQueue_.front();

					if (info.isNewLine)
					{
						time(&rawtime);
						timeinfo = localtime(&rawtime);
						strftime(timeBuf, 10, "%T", timeinfo);

						info.logger->logFile_ << "[" << timeBuf << "] ";
					}

					info.logger->logFile_ << info.data;
					info.logger->logFile_.flush();
					lock.lock();
					logQueue_.pop();
					lock.unlock();
				}
			}
		}));
	}

	Logger::~Logger()
	{
		if (logFile_.is_open())
			logFile_.close();
	}

	void Logger::logRest(const char* str) 
	{
		printf("%s", str);
		forward(str);
	}

	void Logger::logRest(std::string& str)
	{
		printf("%s", str.c_str());
		forward(str.c_str());
	}

	void Logger::log(const char* str)
	{
		char buf[128];
		sprintf(buf, "%s\n", str);
		printf("%s\n", buf);
		forward(buf);
	}

	void Logger::log(std::string& str)
	{
		char buf[128];
		sprintf(buf, "%s\n", str.c_str());
		printf("%s\n", buf);
		forward(buf);
	}

	void Logger::forward(const char* str, bool newLine)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		Logger::LogInfo info = {
			.isNewLine = newLine,
			.logger = this,
			.data = str
		};
		logQueue_.push(info);
		cv_.notify_one();
	}

	void Logger::forward(std::string& str, bool newLine)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		Logger::LogInfo info = {
			.isNewLine = newLine,
			.logger = this,
			.data = str
		};
		logQueue_.push(info);
		cv_.notify_one();
	}
};