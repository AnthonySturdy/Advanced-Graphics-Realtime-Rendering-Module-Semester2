#pragma once
#include "TransformComponent.h"
#include "Game/GameObject.h"

class MouseLookComponent : public Component
{
	enum class ECameraControlType
	{
		FIRST_PERSON = 0,
		THIRD_PERSON
	};

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

	void SetTarget(TransformComponent* t) { TargetTransform = t; }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Mouse Look Controller"; }

private:
	float MouseSensitivity{ 0.02f };
	ECameraControlType CameraControlType{ ECameraControlType::FIRST_PERSON };
	float CameraDistance{ 2.0f };
	float CameraHeight{ 0.2f };

	TransformComponent* TargetTransform{ nullptr };
};
