#pragma once
#include "Game/GameObject.h"

class TransformComponent final : public Component
{
public:
	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(TransformComponent&&) = default;
	TransformComponent& operator=(const TransformComponent&) = default;
	TransformComponent& operator=(TransformComponent&&) = default;
	~TransformComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override {};

	[[nodiscard]] DirectX::SimpleMath::Vector3 GetPosition() const { return Position; }
	[[nodiscard]] DirectX::SimpleMath::Vector3 GetRotation() const { return Rotation; }
	[[nodiscard]] DirectX::SimpleMath::Vector3 GetScale() const { return Scale; }

	[[nodiscard]] DirectX::SimpleMath::Matrix GetWorldMatrix() const;

	void SetPosition(DirectX::SimpleMath::Vector3 val) { Position = val; }
	void SetRotation(DirectX::SimpleMath::Vector3 val) { Rotation = val; }
	void SetScale(DirectX::SimpleMath::Vector3 val) { Scale = val; }

private:
	DirectX::SimpleMath::Vector3 Position{ DirectX::SimpleMath::Vector3::Zero };
	DirectX::SimpleMath::Vector3 Rotation{ DirectX::SimpleMath::Vector3::Zero };
	DirectX::SimpleMath::Vector3 Scale{ DirectX::SimpleMath::Vector3::One };
};
