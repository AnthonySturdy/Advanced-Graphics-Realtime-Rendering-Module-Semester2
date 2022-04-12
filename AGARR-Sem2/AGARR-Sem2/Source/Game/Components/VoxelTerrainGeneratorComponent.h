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
};
