#pragma once
#include "Game/GameObject.h"

class MouseLookComponent : public Component
{
public:
	MouseLookComponent() = default;
	MouseLookComponent(const MouseLookComponent&) = default;
	MouseLookComponent(MouseLookComponent&&) = default;
	MouseLookComponent& operator=(const MouseLookComponent&) = default;
	MouseLookComponent& operator=(MouseLookComponent&&) = default;
	~MouseLookComponent() override = default;

	void Update(float deltaTime) override;
	void Render() override {};
	void RenderGUI() override;

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Mouse Look Controller"; }

private:
	float MouseSensitivity{ 0.02f };
};
