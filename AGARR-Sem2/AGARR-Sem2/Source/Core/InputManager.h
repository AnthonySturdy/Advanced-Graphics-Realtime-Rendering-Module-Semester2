#pragma once
class InputManager
{
public:
	InputManager();
	InputManager(const InputManager&) = default;
	InputManager(InputManager&&) = default;
	InputManager& operator=(const InputManager&) = default;
	InputManager& operator=(InputManager&&) = default;
	~InputManager() = default;

	[[nodiscard]] static DirectX::Keyboard* GetKeyboard()
	{
		if (!Instance)
			Instance = new InputManager();

		return Instance->Keyboard.get();
	}

	[[nodiscard]] static DirectX::Mouse* GetMouse()
	{
		if (!Instance)
			Instance = new InputManager();

		return Instance->Mouse.get();
	}

private:
	static inline InputManager* Instance{ nullptr };

	std::shared_ptr<DirectX::Keyboard> Keyboard{};
	std::shared_ptr<DirectX::Mouse> Mouse{};
};
