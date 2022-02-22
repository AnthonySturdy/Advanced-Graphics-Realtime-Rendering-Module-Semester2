#include "pch.h"
#include "Game/Components/TransformComponent.h"

DirectX::SimpleMath::Matrix TransformComponent::GetWorldMatrix() const
{
	const DirectX::SimpleMath::Matrix tra = XMMatrixTranslationFromVector(Position);
	const DirectX::SimpleMath::Matrix rot = XMMatrixRotationRollPitchYawFromVector(Rotation);
	const DirectX::SimpleMath::Matrix sca = XMMatrixScalingFromVector(Scale);

	return sca * rot * tra;
}
