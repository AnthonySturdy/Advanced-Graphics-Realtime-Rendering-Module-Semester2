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
		const DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(static_cast<float>(mouseState.x),
		                                                                        static_cast<float>(mouseState.y),
		                                                                        0.f) * MouseSensitivity;

		rot.x += delta.x;
		rot.y -= delta.y;

		static constexpr float limit = DirectX::XM_PIDIV2 - 0.01f;
		rot.y = std::clamp(rot.y, -limit, limit);
	}

	transform->SetRotation(rot);
}

void MouseLookComponent::RenderGUI()
{
	ImGui::DragFloat("Mouse Sensitivity", &MouseSensitivity, 0.005f, 0.005f, 10.0f);
}
