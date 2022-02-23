#include "pch.h"
#include "CameraComponent.h"

#include "Game/Components/TransformComponent.h"

CameraComponent::CameraComponent()
	: FOV(DirectX::XM_PIDIV2),
	  NearPlane(0.01f),
	  FarPlane(200.0f)
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

	context->UpdateSubresource(ConstantBuffer.Get(), 0, nullptr, &ccb, 0, 0);

	context->VSSetConstantBuffers(0, 1, ConstantBuffer.GetAddressOf());
}

void CameraComponent::RenderGUI()
{
	ImGui::SliderFloat("FOV", &FOV, 1.0f * 0.0174533, 160.0f * 0.0174533);
	ImGui::DragFloat("Near Plane", &NearPlane, 0.001f, 0.001f, FarPlane - 0.001f);
	ImGui::DragFloat("Far Plane", &FarPlane, 0.001f, NearPlane + 0.001f, 5000.0f);
}

DirectX::SimpleMath::Matrix CameraComponent::GetViewMatrix() const
{
	const TransformComponent* transform = Parent->GetComponent<TransformComponent>();

	const DirectX::SimpleMath::Vector3 eye = transform->GetPosition();
	const DirectX::SimpleMath::Vector3 dir = transform->GetRotation();
	const DirectX::SimpleMath::Vector3 up = DirectX::SimpleMath::Vector3::Up;

	return DirectX::XMMatrixLookAtLH(eye, dir, up);
}

DirectX::SimpleMath::Matrix CameraComponent::GetProjectionMatrix() const
{
	return DirectX::XMMatrixPerspectiveFovLH(FOV, DX::DeviceResources::Instance()->GetViewportAspectRatio(), NearPlane, FarPlane);
}
