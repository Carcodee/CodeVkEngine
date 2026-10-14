//

// Created by carlo on 2024-11-20.
//

#ifndef LOGGER_HPP
#define LOGGER_HPP

namespace systems
{
enum class log_level
{
	l_none  = 0,
	l_error = 1,
	l_warn  = 2,
	l_info  = 3,
	l_debug = 4
};

enum class log_output
{
	console,
	file
};

/**
 * Logger Class Used to Output Details of Current Application Flow
 */
class logger
{
  public:
	void set_log_preferences(log_level    level       = log_level::l_error,
	                       std::string log_file_name = "",
	                       log_output   output      = log_output::console)
	{
		log_level  = level;
		log_output = output;

		if (log_output == log_output::file && !log_file_name.empty())
		{
			log_file.open(log_file_name);
			if (!log_file.good())
			{
				std::cerr << "Can't Open Log File" << std::endl;
				log_output = log_output::console;
			}
		}
	}
	static std::shared_ptr<logger> get_instance()
	{
		if (logger_instance == nullptr)
		{
			logger_instance = std::shared_ptr<logger>(new logger());
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
	void log(std::string message, log_level message_level = log_level::l_debug, std::string code_file = "", int code_line = 0)
	{
		if (message_level <= log_level)
		{
			std::string log_type;
			// Set Log Level Name
			switch (message_level)
			{
				case log_level::l_debug:
					log_type = "DEBUG: ";
					break;
				case log_level::l_info:
					log_type = "INFO: ";
					break;
				case log_level::l_warn:
					log_type = "WARN: ";
					break;
				case log_level::l_error:
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

			log_message(message);
		}
	}
	log_level get_log_level(const std::string &log_level)
	{
		if (log_level == "DEBUG")
		{
			return log_level::l_debug;
		}
		else if (log_level == "INFO")
		{
			return log_level::l_info;
		}
		else if (log_level == "WARN")
		{
			return log_level::l_error;
		}
		else if (log_level == "L_ERROR")
		{
			return log_level::l_error;
		}
		return log_level::l_none;
	}

	log_output get_log_output(const std::string &log_output)
	{
		if (log_output == "FILE")
		{
			return log_output::file;
		}
		// If corrupted string passed output will be on console
		return log_output::console;
	}

	void log_message(const std::string &message)
	{
		if (log_output == log_output::file)
		{
			log_file << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

	void log_vec(const glm::vec3 vec)
	{
		std::string message = "Position: (" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
		if (log_output == log_output::file)
		{
			log_file << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

	void log_vec(const glm::vec2 vec)
	{
		std::string message = "Position: (" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
		if (log_output == log_output::file)
		{
			log_file << message << std::endl;
		}
		else
		{
			std::cout << message << std::endl;
		}
	}

  private:
	log_level      log_level;
	log_output     log_output;
	std::ofstream log_file;

	static std::shared_ptr<logger> logger_instance;
};

std::shared_ptr<logger> logger::logger_instance = nullptr;

}        // namespace systems

#endif        // LOGGER_HPP
