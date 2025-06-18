//
// Created by carlo on 2024-10-24.
//

#ifndef INPUTSYSTEM_HPP
#define INPUTSYSTEM_HPP

namespace SYSTEMS
{
// template singleton for imput system, why is this here? :D
class InputSystem
{
	InputSystem();
	static InputSystem *GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new InputSystem;
		}
		return instance;
	}
	static InputSystem *instance;
};

InputSystem *InputSystem::instance = nullptr;

}        // namespace SYSTEMS

#endif        // INPUTSYSTEM_HPP
