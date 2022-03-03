#include "pch.h"
#include "Game/Components/TransformComponent.h"

void TransformComponent::RenderGUI()
{
	ImGui::PushID(this);

	ImGui::DragFloat3("Position", &Position.x, 0.1f);
	if (ImGui::Button("Reset Position"))
		Position = DirectX::SimpleMath::Vector3::Zero;
	ImGui::DragFloat3("Rotation", &Rotation.x, 0.001f);
	if (ImGui::Button("Reset Rotation"))
		Rotation = DirectX::SimpleMath::Vector3::Zero;
	ImGui::DragFloat3("Scale", &Scale.x, 0.1f);
	if (ImGui::Button("Reset Scale"))
		Scale = DirectX::SimpleMath::Vector3::One;

	ImGui::PopID();
}

DirectX::SimpleMath::Matrix TransformComponent::GetWorldMatrix() const
{
	const DirectX::SimpleMath::Matrix tra = XMMatrixTranslationFromVector(Position);
	const DirectX::SimpleMath::Matrix rot = XMMatrixRotationRollPitchYawFromVector(Rotation);
	const DirectX::SimpleMath::Matrix sca = XMMatrixScalingFromVector(Scale);

	return sca * rot * tra;
}
