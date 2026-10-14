//

// Created by carlo on 2024-11-20.
//

#ifndef LOGGER_HPP
#define LOGGER_HPP

namespace systems
{
enum class Log_Level
{
	L_None  = 0,
	L_Error = 1,
	L_Warn  = 2,
	L_Info  = 3,
	L_Debug = 4
};

enum class Log_Output
{
	Console,
	File
};

/**
 * Logger Class Used to Output Details of Current Application Flow
 */
class Logger
{
  public:
	void Set_Log_Preferences(Log_Level    level       = Log_Level::L_Error,
	                       std::string log_file_name = "",
	                       Log_Output   output      = Log_Output::Console)
	{
		log_level  = level;
		log_output = output;

		if (log_output == Log_Output::File && !log_file_name.empty())
		{
			log_file.open(log_file_name);
			if (!log_file.good())
			{
				std::cerr << "Can't Open Log File" << std::endl;
				log_output = Log_Output::Console;
			}
		}
	}
	static std::shared_ptr<Logger> Get_Instance()
	{
		if (logger_instance == nullptr)
		{
			logger_instance = std::shared_ptr<Logger>(new Logger());
		}

		return logger_instance;
	}

	/**
	 * log given message with defined parameters and generate message to pass on Console or File
	 * @param code_file: __FILE__
	 * @param code_line: __LINE__
	 * @param message: log Message
	 * @param message_level: log Level, log_level::DEBUG by default
	 */
	void Log(std::string message, Log_Level message_level = Log_Level::L_Debug, std::string code_file = "", int code_line = 0)
	{
		if (message_level <= log_level)
		{
			std::string log_type;
			// Set Log Level Name
			switch (message_level)
			{
				case Log_Level::L_Debug:
					log_type = "DEBUG: ";
					break;
				case Log_Level::L_Info:
					log_type = "INFO: ";
					break;
				case Log_Level::L_Warn:
					log_type = "WARN: ";
					break;
				case Log_Level::L_Error:
					log_type = "L_ERROR: ";
					break;
				default:
					log_type = "NONE: ";
					break;
			}
			if (code_line == -1)
			{
				message = log_type + message;
			}
			else
			{
				code_file += " : " + std::to_string(code_line) + " : ";
				message = log_type + code_file + message;
			}

			Log_Message(message);
		}
	}
	Log_Level Get_Log_Level(const std::string &log_level)
	{
		if (log_level == "DEBUG")
		{
			return Log_Level::L_Debug;
		}
		else if (log_level == "INFO")
		{
			return Log_Level::L_Info;
		}
		else if (log_level == "WARN")
		{
			return Log_Level::L_Error;
		}
		else if (log_level == "L_ERROR")
		{
			return Log_Level::L_Error;
		}
		return Log_Level::L_None;
	}

	Log_Output Get_Log_Output(const std::string &log_output)
	{
		if (log_output == "FILE")
		{
			return Log_Output::File;
		}
		// If corrupted string passed output will be on console
		return Log_Output::Console;
	}

	void Log_Message(const std::string &message)
	{
		if (log_output == Log_Output::File)
		{
			log_file << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

	void Log_Vec(const glm::vec3 vec)
	{
		std::string message = "Position: (" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
		if (log_output == Log_Output::File)
		{
			log_file << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

	void Log_Vec(const glm::vec2 vec)
	{
		std::string message = "Position: (" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
		if (log_output == Log_Output::File)
		{
			log_file << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

  private:
	Log_Level      log_level;
	Log_Output     log_output;
	std::ofstream log_file;

	static std::shared_ptr<Logger> logger_instance;
};

std::shared_ptr<Logger> Logger::logger_instance = nullptr;

}        // namespace systems

#endif        // LOGGER_HPP
