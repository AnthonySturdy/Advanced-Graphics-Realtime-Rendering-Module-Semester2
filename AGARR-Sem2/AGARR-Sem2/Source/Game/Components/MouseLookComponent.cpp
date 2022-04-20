#include "pch.h"
#include "MouseLookComponent.h"

#include "TransformComponent.h"
#include "Core/InputManager.h"

void MouseLookComponent::Update(float deltaTime)
{
	const auto mouse = InputManager::GetMouse();
	const auto mouseState = mouse->GetState();
	const auto transform = Parent->GetComponent<TransformComponent>();

	DirectX::SimpleMath::Vector3 rot = transform->GetRotation();

	// Mouse is set to relative upon viewport click
	if (mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		// Calculate rotation
		const DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(static_cast<float>(mouseState.x),
		                                                                        static_cast<float>(mouseState.y),
		                                                                        0.f) * MouseSensitivity;

		rot.x -= delta.y;
		rot.y += delta.x;

		static constexpr float limit = DirectX::XM_PIDIV2 - 0.01f;
		rot.x = std::clamp(rot.x, -limit, limit);
	}

	// Calculate position
	const DirectX::SimpleMath::Vector3 targetPos = TargetTransform ? TargetTransform->GetPosition() : DirectX::SimpleMath::Vector3::Zero;
	const DirectX::SimpleMath::Vector3 offset{
		CameraControlType == ECameraControlType::FIRST_PERSON ? 0.0f : sin(rot.y),
		CameraHeight,
		CameraControlType == ECameraControlType::FIRST_PERSON ? 0.0f : cos(rot.y)
	};
	const DirectX::SimpleMath::Vector3 pos = targetPos + offset * CameraDistance;

	TargetTransform->SetRotation(rot * DirectX::SimpleMath::Vector3::UnitY);
	transform->SetRotation(rot);
	transform->SetPosition(pos);
}

void MouseLookComponent::RenderGUI()
{
	ImGui::DragFloat("Mouse Sensitivity", &MouseSensitivity, 0.005f, 0.005f, 10.0f);

	const char* items[] = { "First Person", "Third Person" };
	static int selection = 0;
	ImGui::Combo("View", &selection, items, 2);
	CameraControlType = static_cast<ECameraControlType>(selection);
	ImGui::DragFloat("Camera Height", &CameraHeight, 0.005f, 0.005f, 50.0f);
	if (selection == 1)
		ImGui::DragFloat("Camera Distance", &CameraDistance, 0.005f, 0.005f, 50.0f);
}
