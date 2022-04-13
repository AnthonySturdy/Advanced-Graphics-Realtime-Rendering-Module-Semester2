#include "pch.h"
#include "TerrainWalkComponent.h"

#include "TransformComponent.h"
#include "Core/InputManager.h"

void TerrainWalkComponent::Update(float deltaTime)
{
	const auto keyboard = InputManager::GetKeyboard();
	const auto keyboardState = keyboard->GetState();
	const auto transform = Parent->GetComponent<TransformComponent>();

	const float camRotY = transform->GetRotation().y;
	const DirectX::XMMATRIX RotateYTempMatrix = DirectX::XMMatrixRotationY(camRotY);

	const DirectX::SimpleMath::Vector3 camRight = XMVector3TransformCoord(DirectX::SimpleMath::Vector3::Right, RotateYTempMatrix);
	const DirectX::SimpleMath::Vector3 camForward = XMVector3TransformCoord(DirectX::SimpleMath::Vector3::Forward, RotateYTempMatrix);

	DirectX::SimpleMath::Vector3 moveDir(0.0f, 0.0f, 0.0f);

	// Vertical (z)
	if (keyboardState.IsKeyDown(DirectX::Keyboard::W))
	{
		moveDir.z = 1.0f;
	}
	else if (keyboardState.IsKeyDown(DirectX::Keyboard::S))
	{
		moveDir.z = -1.0f;
	}

	// Horizontal (x)
	if (keyboardState.IsKeyDown(DirectX::Keyboard::A))
	{
		moveDir.x = 1.0f;
	}
	else if (keyboardState.IsKeyDown(DirectX::Keyboard::D))
	{
		moveDir.x = -1.0f;
	}

	auto curPos = transform->GetPosition();
	curPos.y = DistanceAboveTerrain;
	if (HeightmapGenerator && PlaneMeshGenerator)
	{
		const DirectX::SimpleMath::Vector2 normPlanePosition((transform->GetPosition().x / (PlaneMeshGenerator->GetPlaneSize().x * PlaneMeshGenerator->GetQuadSize())),
		                                                     (transform->GetPosition().z / (PlaneMeshGenerator->GetPlaneSize().y * PlaneMeshGenerator->GetQuadSize())));
		const float heightmapHeight = (static_cast<float>(HeightmapGenerator->SampleHeightmap(normPlanePosition.x, normPlanePosition.y)) / 255.0f) * PlaneMeshGenerator->GetHeightmapVerticalScale();
		curPos.y += heightmapHeight;
	}
	transform->SetPosition(curPos + (camForward * moveDir.z + camRight * moveDir.x) * WalkSpeed);
}

void TerrainWalkComponent::RenderGUI()
{
	ImGui::DragFloat("Player Height", &DistanceAboveTerrain, 0.01f);
	ImGui::DragFloat("Walk Speed", &WalkSpeed, 0.01f);
}
