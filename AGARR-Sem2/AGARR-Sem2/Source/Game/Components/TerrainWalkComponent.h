#pragma once
#include "Game/GameObject.h"
#include "PlaneMeshGeneratorComponent.h"

class TerrainWalkComponent : public Component
{
public:
	TerrainWalkComponent() = default;
	TerrainWalkComponent(const TerrainWalkComponent&) = default;
	TerrainWalkComponent(TerrainWalkComponent&&) = default;
	TerrainWalkComponent& operator=(const TerrainWalkComponent&) = default;
	TerrainWalkComponent& operator=(TerrainWalkComponent&&) = default;
	~TerrainWalkComponent() override = default;

	void SetPlaneGenerator(PlaneMeshGeneratorComponent* planeGenComp) { PlaneMeshGenerator = planeGenComp; }

	void Update(float deltaTime) override;
	void Render() override {};
	void RenderGUI() override;

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Terrain Walk"; }

private:
	PlaneMeshGeneratorComponent* PlaneMeshGenerator{ nullptr };

	float DistanceAboveTerrain{ 1.0f };
	float WalkSpeed{ .3f };
};
