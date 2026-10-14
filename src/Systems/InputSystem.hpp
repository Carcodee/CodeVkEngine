//
// Created by carlo on 2024-10-24.
//

#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

namespace systems
{
// template singleton for imput system, why is this here? :D
class input_system
{
	input_system();
	static input_system *get_instance()
	{
		if (instance == nullptr)
		{
			instance = new input_system;
		}
		return instance;
	}
	static input_system *instance;
};

input_system *input_system::instance = nullptr;

}        // namespace systems

#endif        // INPUT_SYSTEM_HPP
