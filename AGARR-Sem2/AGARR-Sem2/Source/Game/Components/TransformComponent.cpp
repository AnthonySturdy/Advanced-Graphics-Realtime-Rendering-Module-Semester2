#include "Source/pch.h"
#include "Source/Game/Components/TransformComponent.h"

DirectX::SimpleMath::Matrix TransformComponent::GetWorldMatrix() const
{
	const DirectX::SimpleMath::Matrix tra = DirectX::XMMatrixTranslationFromVector(Position);
	const DirectX::SimpleMath::Matrix rot = DirectX::XMMatrixRotationRollPitchYawFromVector(Rotation);
	const DirectX::SimpleMath::Matrix sca = DirectX::XMMatrixScalingFromVector(Scale);

	return sca * rot * tra;
}
