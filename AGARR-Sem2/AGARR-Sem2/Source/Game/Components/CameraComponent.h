#pragma once
#include "Game/GameObject.h"

class CameraComponent : public Component
{
	struct CameraConstantBuffer
	{
		DirectX::SimpleMath::Matrix View{ DirectX::SimpleMath::Matrix::Identity };
		DirectX::SimpleMath::Matrix Projection{ DirectX::SimpleMath::Matrix::Identity };
		DirectX::SimpleMath::Vector3 EyePos{ DirectX::SimpleMath::Vector3::Zero };
		float padding{};
	};

public:
	CameraComponent();
	CameraComponent(float fov, float nearPlane, float farPlane);
	CameraComponent(const CameraComponent&) = default;
	CameraComponent(CameraComponent&&) = default;
	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;
	~CameraComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] float GetFOV() const { return FOV; }
	[[nodiscard]] float GetNearPlane() const { return NearPlane; }
	[[nodiscard]] float GetFarPlane() const { return FarPlane; }

	[[nodiscard]] DirectX::SimpleMath::Matrix GetViewMatrix() const;
	[[nodiscard]] DirectX::SimpleMath::Matrix GetProjectionMatrix() const;

	void SetFOV(float val) { FOV = val; }
	void SetNearPlane(float val) { NearPlane = val; }
	void SetFarPlane(float val) { FarPlane = val; }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Camera"; }

private:
	void CreateConstantBuffer();

	float FOV;
	float NearPlane;
	float FarPlane;

	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
};
