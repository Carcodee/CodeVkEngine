//

// Created by carlo on 2024-11-20.
//

#ifndef LOGGERSYSTEM_HPP
#define LOGGERSYSTEM_HPP

namespace SYSTEMS
{
enum class LogLevel
{
	L_NONE  = 0,
	L_ERROR = 1,
	L_WARN  = 2,
	L_INFO  = 3,
	L_DEBUG = 4
};

enum class LogOutput
{
	CONSOLE,
	FILE
};

/**
 * Logger Class Used to Output Details of Current Application Flow
 */
class Logger
{
  public:
	void SetLogPreferences(LogLevel    level       = LogLevel::L_ERROR,
	                       std::string logFileName = "",
	                       LogOutput   output      = LogOutput::CONSOLE)
	{
		logLevel  = level;
		logOutput = output;

		if (logOutput == LogOutput::FILE && !logFileName.empty())
		{
			logFile.open(logFileName);
			if (!logFile.good())
			{
				std::cerr << "Can't Open Log File" << std::endl;
				logOutput = LogOutput::CONSOLE;
			}
		}
	}
	static std::shared_ptr<Logger> GetInstance()
	{
		if (loggerInstance == nullptr)
		{
			loggerInstance = std::shared_ptr<Logger>(new Logger());
		}

		return loggerInstance;
	}

	/**
	 * Log given message with defined parameters and generate message to pass on Console or File
	 * @param codeFile: __FILE__
	 * @param codeLine: __LINE__
	 * @param message: Log Message
	 * @param messageLevel: Log Level, LogLevel::DEBUG by default
	 */
	void Log(std::string message, LogLevel messageLevel = LogLevel::L_DEBUG, std::string codeFile = "", int codeLine = 0)
	{
		if (messageLevel <= logLevel)
		{
			std::string logType;
			// Set Log Level Name
			switch (messageLevel)
			{
				case LogLevel::L_DEBUG:
					logType = "DEBUG: ";
					break;
				case LogLevel::L_INFO:
					logType = "INFO: ";
					break;
				case LogLevel::L_WARN:
					logType = "WARN: ";
					break;
				case LogLevel::L_ERROR:
					logType = "L_ERROR: ";
					break;
				default:
					logType = "NONE: ";
					break;
			}
			if (codeLine == -1)
			{
				message = logType + message;
			}
			else
			{
				codeFile += " : " + std::to_string(codeLine) + " : ";
				message = logType + codeFile + message;
			}

			LogMessage(message);
		}
	}
	LogLevel GetLogLevel(const std::string &logLevel)
	{
		if (logLevel == "DEBUG")
		{
			return LogLevel::L_DEBUG;
		}
		else if (logLevel == "INFO")
		{
			return LogLevel::L_INFO;
		}
		else if (logLevel == "WARN")
		{
			return LogLevel::L_ERROR;
		}
		else if (logLevel == "L_ERROR")
		{
			return LogLevel::L_ERROR;
		}
		return LogLevel::L_NONE;
	}

	LogOutput GetLogOutput(const std::string &logOutput)
	{
		if (logOutput == "FILE")
		{
			return LogOutput::FILE;
		}
		// If corrupted string passed output will be on console
		return LogOutput::CONSOLE;
	}

	void LogMessage(const std::string &message)
	{
		if (logOutput == LogOutput::FILE)
		{
			logFile << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

	void LogVec(const glm::vec3 vec)
	{
		std::string message = "Position: (" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
		if (logOutput == LogOutput::FILE)
		{
			logFile << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

	void LogVec(const glm::vec2 vec)
	{
		std::string message = "Position: (" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
		if (logOutput == LogOutput::FILE)
		{
			logFile << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

  private:
	LogLevel      logLevel;
	LogOutput     logOutput;
	std::ofstream logFile;

	static std::shared_ptr<Logger> loggerInstance;
};

std::shared_ptr<Logger> Logger::loggerInstance = nullptr;

}        // namespace SYSTEMS

#endif        // LOGGERSYSTEM_HPP
