#include "pch.h"
#include "InputManager.h"

InputManager::InputManager()
{
	Keyboard = std::make_shared<DirectX::Keyboard>();
	Mouse = std::make_shared<DirectX::Mouse>();
}
