//
// Created by carlo on 2024-10-24.
//

#ifndef INPUT_SYSTEM_HPP
#define INPUT_SYSTEM_HPP

namespace systems
{
// template singleton for imput system, why is this here? :D
class Input_System
{
	Input_System();
	static Input_System *Get_Instance()
	{
		if (instance == nullptr)
		{
			instance = new Input_System;
		}
		return instance;
	}
	static Input_System *instance;
};

Input_System *Input_System::instance = nullptr;

}        // namespace systems

#endif        // INPUT_SYSTEM_HPP
