#pragma once
#include "Game/Components/HeightmapGeneratorComponent.h"

class VoxelTerrainGeneratorComponent : public Component
{
public:
	VoxelTerrainGeneratorComponent() = default;
	VoxelTerrainGeneratorComponent(const VoxelTerrainGeneratorComponent&) = default;
	VoxelTerrainGeneratorComponent(VoxelTerrainGeneratorComponent&&) = default;
	VoxelTerrainGeneratorComponent& operator=(const VoxelTerrainGeneratorComponent&) = default;
	VoxelTerrainGeneratorComponent& operator=(VoxelTerrainGeneratorComponent&&) = default;
	~VoxelTerrainGeneratorComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override;
	void RenderGUI() override;

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Voxel Mesh Generator"; }

private:
	void GenerateVoxelData();
	void GenerateVoxelMesh();

	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateFaceNormal(bool isBackFace, int axis) const;

	[[nodiscard]] int Index(const int x, const int y, const int z) const { return (z * VolumeSize[0] * VolumeSize[1]) + (y * VolumeSize[0]) + x; }

	int VolumeSize[3]{ 64, 64, 64 };
	uint8_t* VoxelData{ nullptr };

	float NormHeightmapVerticalScale{ 0.3f };
	float NormHeightmapPosition{ 0.7f };
};
