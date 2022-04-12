#include "pch.h"
#include "PlaneTerrainMeshGeneratorComponent.h"

#include "HeightmapGeneratorComponent.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "Rendering/Mesh.h"

PlaneTerrainMeshGeneratorComponent::PlaneTerrainMeshGeneratorComponent()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TerrainConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, TerrainCBuffer.ReleaseAndGetAddressOf()));
}

void PlaneTerrainMeshGeneratorComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	const auto heightmapGenerator = Parent->GetComponent<HeightmapGeneratorComponent>();
	if (!heightmapGenerator)
		return;

	// Update shader resources
	context->DSSetShaderResources(0, 1, heightmapGenerator->GetHeightmapSRV());

	TerrainCBufferData.NumTextures = Parent->GetComponent<MaterialComponent>()->GetNumTextures();
	TerrainCBufferData.ApplyHeightmap = UseHeightmap;
	TerrainCBufferData.HeightmapScale = HeightmapVerticalScale * UseHeightmap;
	context->UpdateSubresource(TerrainCBuffer.Get(), 0, nullptr, &TerrainCBufferData, 0, 0);
	context->DSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
	context->PSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
}

void PlaneTerrainMeshGeneratorComponent::RenderGUI()
{
	// Heightmap controls
	ImGui::Checkbox("Use Heightmap", &UseHeightmap);
	if (UseHeightmap)
	{
		ImGui::DragFloat("Heightmap Scale", &HeightmapVerticalScale, 0.01f, 0.01f, 500.0f);
	}

	// Plane Size
	ImGui::DragInt2("Plane Size", &PlaneSize[0], 1, 1, INT16_MAX);
	ImGui::DragInt("Quad Size", &QuadSize, 1.0f, 1);

	// Plane generation
	if (ImGui::Button("Generate Plane"))
	{
		GenerateMesh(PlaneSize[0], PlaneSize[1]);
	}
}

void PlaneTerrainMeshGeneratorComponent::GenerateMesh(int width, int height)
{
	const auto meshRenderer = Parent->GetComponent<MeshRendererComponent>();
	if (!meshRenderer)
		return;

	++width;
	++height;

	std::vector<Vertex> newVerts;
	std::vector<UINT> newIndices;
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const float normX = static_cast<float>(x) / width;
			const float normY = static_cast<float>(y) / height;

			const float worldX = x * QuadSize;
			const float worldY = y * QuadSize;

			// Generate vertices
			Vertex vert = {
				DirectX::SimpleMath::Vector3(worldX, 0, worldY),
				DirectX::SimpleMath::Vector3::Up,
				DirectX::SimpleMath::Vector2(normX, normY)
			};
			newVerts.push_back(vert);

			// Generate indices
			if (x == width - 1 || y == height - 1)
				continue;
			//Triangle 1
			newIndices.push_back(y * width + x);
			newIndices.push_back((y + 1) * width + x);
			newIndices.push_back(y * width + (x + 1));

			//Triangle 2
			newIndices.push_back(y * width + (x + 1));
			newIndices.push_back((y + 1) * width + x);
			newIndices.push_back((y + 1) * width + (x + 1));
		}
	}

	Mesh* mesh = meshRenderer->GetMesh();
	mesh->Initialise(newVerts, newIndices);
}
