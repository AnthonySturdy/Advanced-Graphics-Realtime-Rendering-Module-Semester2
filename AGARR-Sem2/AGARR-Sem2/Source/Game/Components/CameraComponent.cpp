#include "pch.h"
#include "CameraComponent.h"

#include "Game/Components/TransformComponent.h"

CameraComponent::CameraComponent()
	: FOV(DirectX::XM_PIDIV2),
	  NearPlane(0.01f),
	  FarPlane(500.0f)
{
	CreateConstantBuffer();
}

CameraComponent::CameraComponent(float fov, float nearPlane, float farPlane)
	: FOV(fov),
	  NearPlane(nearPlane),
	  FarPlane(farPlane)
{
	CreateConstantBuffer();
}

void CameraComponent::CreateConstantBuffer()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CameraConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, ConstantBuffer.ReleaseAndGetAddressOf()));
}

void CameraComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	static CameraConstantBuffer ccb = {};
	ccb.View = XMMatrixTranspose(GetViewMatrix());
	ccb.Projection = XMMatrixTranspose(GetProjectionMatrix());
	ccb.EyePos = Parent->GetComponent<TransformComponent>()->GetPosition();

	context->UpdateSubresource(ConstantBuffer.Get(), 0, nullptr, &ccb, 0, 0);

	context->VSSetConstantBuffers(0, 1, ConstantBuffer.GetAddressOf());
	context->DSSetConstantBuffers(0, 1, ConstantBuffer.GetAddressOf());
}

void CameraComponent::RenderGUI()
{
	ImGui::SliderFloat("FOV", &FOV, 1.0f * 0.0174533, 160.0f * 0.0174533);
	ImGui::DragFloat("Near Plane", &NearPlane, 0.001f, 0.001f, FarPlane - 0.001f);
	ImGui::DragFloat("Far Plane", &FarPlane, 0.001f, NearPlane + 0.001f, 5000.0f);
}

DirectX::SimpleMath::Matrix CameraComponent::GetViewMatrix() const
{
	// Modified version of: https://www.braynzarsoft.net/viewtutorial/q16390-19-first-person-camera
	// Convert euler rotation to view matrix

	const TransformComponent* transform = Parent->GetComponent<TransformComponent>();

	const DirectX::SimpleMath::Vector3 eye = transform->GetPosition();
	const DirectX::SimpleMath::Vector3 dir = transform->GetRotation();

	// Pitch and Yaw
	const DirectX::SimpleMath::Matrix camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(dir.x, dir.y, 0);
	DirectX::SimpleMath::Vector3 camTarget = DirectX::XMVector3TransformCoord(DirectX::SimpleMath::Vector3::Forward, camRotationMatrix);
	camTarget.Normalize();
	camTarget += eye;

	// Roll
	const DirectX::SimpleMath::Matrix camYRotationMatrix = DirectX::XMMatrixRotationZ(dir.z);
	const DirectX::SimpleMath::Vector3 camUp = DirectX::XMVector3TransformCoord(DirectX::SimpleMath::Vector3::Up, camYRotationMatrix);

	return DirectX::XMMatrixLookAtLH(eye, camTarget, camUp);
}

DirectX::SimpleMath::Matrix CameraComponent::GetProjectionMatrix() const
{
	return DirectX::XMMatrixPerspectiveFovLH(FOV, DX::DeviceResources::Instance()->GetViewportAspectRatio(), NearPlane, FarPlane);
}
