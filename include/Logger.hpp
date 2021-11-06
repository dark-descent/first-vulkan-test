#ifndef ENGINE_LOGGER_HPP
#define ENGINE_LOGGER_HPP

#include "framework.hpp"

namespace NovaEngine
{
	class Logger
	{
	private:
		struct LogInfo
		{
			Logger* logger;
			std::string data;
		};

		static std::optional<std::thread> logHandlerThread_;
		static std::mutex mutex_;
		static std::condition_variable cv_;

		static std::unordered_map<std::string, Logger*> loggers_;
		static std::queue<LogInfo> logQueue_;

		static bool shouldTerminate_;

		static inline std::string& date();

	public:
		static Logger* get(const char* name = nullptr);

		static void terminate();

	private:
		static const char* DEFAULT_COLOR;
		static const char* INFO_COLOR;
		static const char* WARN_COLOR;
		static const char* ERROR_COLOR;

		std::string path_;
		std::ofstream logFile_;

	public:
		Logger(const char* path);
		~Logger();

		enum class Severity
		{
			DEFAULT,
			INFO,
			WARNING,
			ERROR,
		};

		void forward(const char* str);
		void forward(std::string& str);

		void logRest(const char* str);
		void logRest(std::string& str);

		template<typename T, typename... Rest>
		void logRest(T str, Rest... rest)
		{
			logRest(str);
			logRest(rest...);
		}

	public:
		void log(const char* str);
		void log(std::string& str);

		template<typename T>
		void log(T str)
		{
			log(str);
		}

		template<typename T, typename... Ts>
		void log(T str, Ts... rest)
		{
			printf("%s", str);
			forward(str);
			log(rest...);
		}

		template<typename T>
		void info(T str)
		{
			printf("%s[INFO]%s: ", INFO_COLOR, DEFAULT_COLOR);
			forward("[INFO] ");
			log(str);
		}

		template<typename T>
		void warn(T str)
		{
			printf("%s[WARN]%s: ", WARN_COLOR, DEFAULT_COLOR);
			forward("[WARN] ");
			log(str);
		}

		template<typename T>
		void error(T str)
		{
			printf("%s[ERROR]%s: ", ERROR_COLOR, DEFAULT_COLOR);
			forward("[ERROR] ");
			log(str);
		}

		template<typename T, typename... Ts>
		void info(T str, Ts... rest)
		{
			printf("%s[INFO]%s: %s", INFO_COLOR, DEFAULT_COLOR, str);
			forward("[INFO] ");
			forward(str);
			logRest(rest...);
			printf("\n");
			forward("\n");
		}

		template<typename T, typename... Ts>
		void warn(T str, Ts... rest)
		{
			printf("%s[WARN]%s: %s", WARN_COLOR, DEFAULT_COLOR, str);
			forward("[WARN] ");
			forward(str);
			logRest(rest...);
			printf("\n");
			forward("\n");
		}

		template<typename T, typename... Ts>
		void error(T str, Ts... rest)
		{
			printf("%s[ERROR]%s: %s", ERROR_COLOR, DEFAULT_COLOR, str);
			forward("[ERROR] ");
			forward(str);
			logRest(rest...);
			printf("\n");
			forward("\n");
		}

		template<typename... Ts>
		void log(Severity type, Ts... rest)
		{
			switch (type)
			{
			case Severity::INFO:
				info(rest...);
				break;
			case Severity::WARNING:
				warn(rest...);
				break;
			case Severity::ERROR:
				error(rest...);
				break;
			case Severity::DEFAULT:
			default:
				log(rest...);
				break;
			}
		}

		template<typename T>
		void log(Severity type, T str)
		{
			switch (type)
			{
			case Severity::INFO:
				info(str);
				break;
			case Severity::WARNING:
				warn(str);
				break;
			case Severity::ERROR:
				error(str);
				break;
			case Severity::DEFAULT:
			default:
				log(str);
				break;
			}
		}
	};
}

#endif
