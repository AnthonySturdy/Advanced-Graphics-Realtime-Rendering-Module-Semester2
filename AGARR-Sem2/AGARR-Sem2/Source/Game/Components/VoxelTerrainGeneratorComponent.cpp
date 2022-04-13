#include "pch.h"
#include "VoxelTerrainGeneratorComponent.h"

#include "MeshRendererComponent.h"
#include "Rendering/Mesh.h"

void VoxelTerrainGeneratorComponent::Render() { }

void VoxelTerrainGeneratorComponent::RenderGUI()
{
	ImGui::DragInt3("Volume Size", &VolumeSize[0], 1, 64, 512);

	if (ImGui::SliderFloat("Heightmap Scale", &NormHeightmapVerticalScale, 0.0f, 1.0f))
		NormHeightmapPosition = 1.0f - NormHeightmapVerticalScale;
	if (ImGui::SliderFloat("Heightmap Position", &NormHeightmapPosition, 0.0f, 1.0f))
		NormHeightmapVerticalScale = 1.0f - NormHeightmapPosition;

	if (ImGui::Button("Generate Voxel Data"))
		GenerateVoxelData();

	if (ImGui::Button("Generate Voxel Mesh"))
		GenerateVoxelMesh();
}

void VoxelTerrainGeneratorComponent::GenerateVoxelData()
{
	const auto heightmapGenerator = Parent->GetComponent<HeightmapGeneratorComponent>();
	if (!heightmapGenerator)
		return;

	delete[] VoxelData;
	VoxelData = new uint8_t[VolumeSize[0] * VolumeSize[1] * VolumeSize[2]];

	for (int x = 0; x < VolumeSize[0]; ++x)
	{
		for (int z = 0; z < VolumeSize[2]; ++z)
		{
			const float nX = x / static_cast<float>(VolumeSize[0]);
			const float nZ = z / static_cast<float>(VolumeSize[2]);

			const float heightmap = heightmapGenerator->SampleHeightmap(nX, nZ) / 255.0f;

			const int terrainHeight = static_cast<int>(VolumeSize[1] * (NormHeightmapPosition + heightmap * NormHeightmapVerticalScale));
			for (int y = 0; y < terrainHeight; ++y)
			{
				VoxelData[Index(x, y, z)] = 1;
			}
		}
	}
}

void VoxelTerrainGeneratorComponent::GenerateVoxelMesh()
{
	const auto meshRenderer = Parent->GetComponent<MeshRendererComponent>();
	if (!meshRenderer)
		return;

	std::vector<UINT> indices{};
	std::vector<Vertex> vertices{};

	for (int x = 0; x < VolumeSize[0]; ++x)
	{
		for (int y = 0; y < VolumeSize[1]; ++y)
		{
			for (int z = 0; z < VolumeSize[2]; ++z)
			{
				if (VoxelData[Index(x, y, z)] == 0)
					continue;

				// Positive X
				if (VoxelData[Index(x + 1, y, z)])
				{
					const int vs = vertices.size();
					vertices.push_back({ { x + 1.0f, y + 0.0f, z + 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 1.0f, z + 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 1.0f, z + 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
					vertices.push_back({ { x + 1.0f, y + 0.0f, z + 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } });

					indices.push_back(vs + 0);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
					indices.push_back(vs + 2);
					indices.push_back(vs + 3);
					indices.push_back(vs + 0);
				}
				// Negative X
				if (VoxelData[Index(x - 1, y, z)])
				{
					const int vs = vertices.size();
					vertices.push_back({ { x + 0.0f, y + 0.0f, z + 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
					vertices.push_back({ { x + 0.0f, y + 1.0f, z + 0.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
					vertices.push_back({ { x + 0.0f, y + 1.0f, z + 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
					vertices.push_back({ { x + 0.0f, y + 0.0f, z + 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } });

					indices.push_back(vs + 0);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
					indices.push_back(vs + 2);
					indices.push_back(vs + 3);
					indices.push_back(vs + 0);
				}

				// Positive Y
				if (VoxelData[Index(x + 1, y, z)])
				{
					const int vs = vertices.size();
					vertices.push_back({ { x + 0.0f, y + 1.0f, z + 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 1.0f, z + 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 1.0f, z + 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } });
					vertices.push_back({ { x + 0.0f, y + 1.0f, z + 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } });

					indices.push_back(vs + 0);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
					indices.push_back(vs + 2);
					indices.push_back(vs + 3);
					indices.push_back(vs + 0);
				}
				// Negative Y
				if (VoxelData[Index(x - 1, y, z)])
				{
					const int vs = vertices.size();
					vertices.push_back({ { x + 0.0f, y + 0.0f, z + 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 0.0f, z + 0.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 0.0f, z + 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } });
					vertices.push_back({ { x + 0.0f, y + 0.0f, z + 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } });

					indices.push_back(vs + 0);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
					indices.push_back(vs + 2);
					indices.push_back(vs + 3);
					indices.push_back(vs + 0);
				}

				// Positive Z
				if (VoxelData[Index(x + 1, y, z)])
				{
					const int vs = vertices.size();
					vertices.push_back({ { x + 0.0f, y + 0.0f, z + 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 0.0f, z + 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 1.0f, z + 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
					vertices.push_back({ { x + 0.0f, y + 1.0f, z + 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });

					indices.push_back(vs + 0);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
					indices.push_back(vs + 2);
					indices.push_back(vs + 3);
					indices.push_back(vs + 0);
				}
				// Negative Z
				if (VoxelData[Index(x - 1, y, z)])
				{
					const int vs = vertices.size();
					vertices.push_back({ { x + 0.0f, y + 0.0f, z + 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 0.0f, z + 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } });
					vertices.push_back({ { x + 1.0f, y + 1.0f, z + 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } });
					vertices.push_back({ { x + 0.0f, y + 1.0f, z + 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } });

					indices.push_back(vs + 0);
					indices.push_back(vs + 1);
					indices.push_back(vs + 2);
					indices.push_back(vs + 2);
					indices.push_back(vs + 3);
					indices.push_back(vs + 0);
				}
			}
		}
	}

	// Send data to Mesh object
	meshRenderer->GetMesh()->Initialise(vertices, indices);
}

DirectX::SimpleMath::Vector3 VoxelTerrainGeneratorComponent::CalculateFaceNormal(const bool isBackFace, const int axis) const
{
	const int backface = (isBackFace ? -1 : 1);
	switch (axis)
	{
	case 0:
		return { 1.0f * backface, 0.0f, 0.0f };

	case 1:
		return { 0.0f, 1.0f * backface, 0.0f };

	case 2:
		return { 0.0f, 0.0f, 1.0f * backface };

	default:
		return { 0.0f, 0.0f, 0.0f };
	}
}
