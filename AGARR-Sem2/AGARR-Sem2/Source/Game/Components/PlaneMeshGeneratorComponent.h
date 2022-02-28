#pragma once
#include "Game/GameObject.h"

class PlaneMeshGeneratorComponent : public Component
{
public:
	PlaneMeshGeneratorComponent() = default;
	PlaneMeshGeneratorComponent(const PlaneMeshGeneratorComponent&) = default;
	PlaneMeshGeneratorComponent(PlaneMeshGeneratorComponent&&) = default;
	PlaneMeshGeneratorComponent& operator=(const PlaneMeshGeneratorComponent&) = default;
	PlaneMeshGeneratorComponent& operator=(PlaneMeshGeneratorComponent&&) = default;
	~PlaneMeshGeneratorComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override {};
	void RenderGUI() override;

protected:
	[[nodiscard]] std::string GetComponentName() const override { return "Plane Mesh Generator"; }

private:
	void GeneratePlane(int width, int height, bool useHeightmap = false, float heightMapScale = 1.0f);
	void LoadHeightmap(const std::wstring& path);
	[[nodiscard]] unsigned char SampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char SampleHeightmap(float normx, float normy) const;
	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(int x, int y) const;
	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(float normx, float normy) const;

	unsigned int HeightmapSize = 0;
	std::vector<unsigned char> Heightmap{};
};
