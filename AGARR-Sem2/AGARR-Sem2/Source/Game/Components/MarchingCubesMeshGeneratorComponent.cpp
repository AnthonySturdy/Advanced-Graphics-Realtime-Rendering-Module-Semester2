#include "pch.h"
#include "MarchingCubesMeshGeneratorComponent.h"

#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "TransformComponent.h"
#include "Rendering/Mesh.h"
#include "Utility/PerlinNoise.h"

MarchingCubesMeshGeneratorComponent::MarchingCubesMeshGeneratorComponent()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TerrainConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, TerrainCBuffer.ReleaseAndGetAddressOf()));
}

void MarchingCubesMeshGeneratorComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	const auto heightmapGenerator = Parent->GetComponent<HeightmapGeneratorComponent>();
	if (!heightmapGenerator)
		return;

	// Update shader resources
	context->DSSetShaderResources(0, 1, heightmapGenerator->GetHeightmapSRV());

	TerrainCBufferData.NumTextures = Parent->GetComponent<MaterialComponent>()->GetNumTextures();
	TerrainCBufferData.ApplyHeightmap = false;
	TerrainCBufferData.HeightmapScale = Parent->GetComponent<TransformComponent>()->GetScale().y;
	context->UpdateSubresource(TerrainCBuffer.Get(), 0, nullptr, &TerrainCBufferData, 0, 0);
	context->DSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
	context->PSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
}

void MarchingCubesMeshGeneratorComponent::RenderGUI()
{
	ImGui::DragFloat("Mesh Resolution", &Resolution, 0.5f, 16.0f, 2048.0f);
	ImGui::DragFloat("Cave Frequency", &CaveFrequency, 0.1f, 0.1f, 100.0f);
	ImGui::SliderFloat("Cave Threshold", &CaveThreshold, 0.0f, 1.0f);

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	if (ImGui::SliderFloat("Heightmap Scale", &NormHeightmapVerticalScale, 0.0f, 1.0f))
		NormHeightmapPosition = 1.0f - NormHeightmapVerticalScale;
	if (ImGui::SliderFloat("Heightmap Position", &NormHeightmapPosition, 0.0f, 1.0f))
		NormHeightmapVerticalScale = 1.0f - NormHeightmapPosition;

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	if (ImGui::Button("Generate Voxel Mesh"))
		GenerateMarchingCubesMesh();
}

bool MarchingCubesMeshGeneratorComponent::SampleTerrain3D(const DirectX::SimpleMath::Vector3& pos) const
{
#define SOLID true
#define AIR false

	const auto heightmapGenerator = Parent->GetComponent<HeightmapGeneratorComponent>();
	if (!heightmapGenerator)
		return AIR;

	const auto perlin = PerlinNoise::noise(pos.x * CaveFrequency, pos.y * CaveFrequency, pos.z * CaveFrequency);
	if (perlin < CaveThreshold)
		return AIR;

	const float heightmap = heightmapGenerator->SampleHeightmap(pos.x, pos.z) / 255.0f;
	const float terrainHeight = NormHeightmapPosition + heightmap * NormHeightmapVerticalScale;
	if (pos.y < terrainHeight)
		return SOLID;

	return AIR;
}

DirectX::SimpleMath::Vector3 MarchingCubesMeshGeneratorComponent::Interpolate(float isolevel, DirectX::SimpleMath::Vector3 p1, DirectX::SimpleMath::Vector3 p2, bool valp1, bool valp2) const
{
	DirectX::SimpleMath::Vector3 p;

	if (std::abs(isolevel - static_cast<float>(valp1)) < 0.001f)
		return p1;
	if (std::abs(isolevel - static_cast<float>(valp2)) < 0.001f)
		return p2;
	if (std::abs(static_cast<float>(valp1) - static_cast<float>(valp2)) < 0.001f)
		return p1;
	const float mu = (isolevel - static_cast<float>(valp1)) / (static_cast<float>(valp2) - static_cast<float>(valp1));
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);

	return p;
}

/***********************************************
MARKING SCHEME: Advanced terrain generation
DESCRIPTION:	Generates mesh from 3D terrain samples using marching cubes algorithm
***********************************************/
void MarchingCubesMeshGeneratorComponent::GenerateMarchingCubesMesh()
{
	const auto meshRenderer = Parent->GetComponent<MeshRendererComponent>();
	if (!meshRenderer)
		return;

	std::vector<UINT> indices{};
	std::vector<Vertex> vertices{};

	const float cellSize = 1.0f / Resolution;

	for (float x = 0; x < 1.0f; x += cellSize)
	{
		for (float y = 0; y < 1.0f; y += cellSize)
		{
			for (float z = 0; z < 1.0f; z += cellSize)
			{
				bool val[8]{};
				DirectX::SimpleMath::Vector3 positions[8]{};
				positions[0] = DirectX::SimpleMath::Vector3{ x, y, z };
				positions[1] = DirectX::SimpleMath::Vector3{ x + cellSize, y, z };
				positions[2] = DirectX::SimpleMath::Vector3{ x + cellSize, y, z + cellSize };
				positions[3] = DirectX::SimpleMath::Vector3{ x, y, z + cellSize };
				positions[4] = DirectX::SimpleMath::Vector3{ x, y + cellSize, z };
				positions[5] = DirectX::SimpleMath::Vector3{ x + cellSize, y + cellSize, z };
				positions[6] = DirectX::SimpleMath::Vector3{ x + cellSize, y + cellSize, z + cellSize };
				positions[7] = DirectX::SimpleMath::Vector3{ x, y + cellSize, z + cellSize };
				val[0] = SampleTerrain3D(positions[0]);
				val[1] = SampleTerrain3D(positions[1]);
				val[2] = SampleTerrain3D(positions[2]);
				val[3] = SampleTerrain3D(positions[3]);
				val[4] = SampleTerrain3D(positions[4]);
				val[5] = SampleTerrain3D(positions[5]);
				val[6] = SampleTerrain3D(positions[6]);
				val[7] = SampleTerrain3D(positions[7]);

				int cubeindex = 0;
				if (val[0]) cubeindex |= 1;
				if (val[1]) cubeindex |= 2;
				if (val[2]) cubeindex |= 4;
				if (val[3]) cubeindex |= 8;
				if (val[4]) cubeindex |= 16;
				if (val[5]) cubeindex |= 32;
				if (val[6]) cubeindex |= 64;
				if (val[7]) cubeindex |= 128;

				/* Cube is entirely in/out of the surface */
				if (edgeTable[cubeindex] == 0)
					continue;

				/* Find the vertices where the surface intersects the cube */
				DirectX::SimpleMath::Vector3 vertlist[12]{};
				if (edgeTable[cubeindex] & 1)
					vertlist[0] = Interpolate(0.5f, positions[0], positions[1], val[0], val[1]);
				if (edgeTable[cubeindex] & 2)
					vertlist[1] = Interpolate(0.5f, positions[1], positions[2], val[1], val[2]);
				if (edgeTable[cubeindex] & 4)
					vertlist[2] = Interpolate(0.5f, positions[2], positions[3], val[2], val[3]);
				if (edgeTable[cubeindex] & 8)
					vertlist[3] = Interpolate(0.5f, positions[3], positions[0], val[3], val[0]);
				if (edgeTable[cubeindex] & 16)
					vertlist[4] = Interpolate(0.5f, positions[4], positions[5], val[4], val[5]);
				if (edgeTable[cubeindex] & 32)
					vertlist[5] = Interpolate(0.5f, positions[5], positions[6], val[5], val[6]);
				if (edgeTable[cubeindex] & 64)
					vertlist[6] = Interpolate(0.5f, positions[6], positions[7], val[6], val[7]);
				if (edgeTable[cubeindex] & 128)
					vertlist[7] = Interpolate(0.5f, positions[7], positions[4], val[7], val[4]);
				if (edgeTable[cubeindex] & 256)
					vertlist[8] = Interpolate(0.5f, positions[0], positions[4], val[0], val[4]);
				if (edgeTable[cubeindex] & 512)
					vertlist[9] = Interpolate(0.5f, positions[1], positions[5], val[1], val[5]);
				if (edgeTable[cubeindex] & 1024)
					vertlist[10] = Interpolate(0.5f, positions[2], positions[6], val[2], val[6]);
				if (edgeTable[cubeindex] & 2048)
					vertlist[11] = Interpolate(0.5f, positions[3], positions[7], val[3], val[7]);

				for (int i = 0; triTable[cubeindex][i] != -1; i += 3)
				{
					const int vs = vertices.size();

					// Calculate normal
					const auto p1 = vertlist[triTable[cubeindex][i]];
					const auto p2 = vertlist[triTable[cubeindex][i + 1]];
					const auto p3 = vertlist[triTable[cubeindex][i + 2]];

					const auto v1 = p2 - p1;
					const auto v2 = p3 - p1;
					DirectX::SimpleMath::Vector3 norm = v1.Cross(v2);
					norm.Normalize();

					// Add vertices
					vertices.push_back({ vertlist[triTable[cubeindex][i]], norm, DirectX::SimpleMath::Vector2::Zero });
					vertices.push_back({ vertlist[triTable[cubeindex][i + 1]], norm, DirectX::SimpleMath::Vector2::Zero });
					vertices.push_back({ vertlist[triTable[cubeindex][i + 2]], norm, DirectX::SimpleMath::Vector2::Zero });

					// Add indices
					indices.push_back(vs);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
				}
			}
		}
	}

	// Send data to Mesh object
	meshRenderer->GetMesh()->Initialise(vertices, indices);
}
