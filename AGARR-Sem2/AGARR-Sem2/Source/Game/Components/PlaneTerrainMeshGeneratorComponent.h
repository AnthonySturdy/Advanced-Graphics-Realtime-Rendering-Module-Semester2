#pragma once
#include "Game/GameObject.h"

struct TerrainConstantBuffer
{
	BOOL ApplyHeightmap{ false };
	float HeightmapScale{ 50.0f };
	unsigned int NumTextures{ 0u };
	float padding{};
};

class PlaneTerrainMeshGeneratorComponent : public Component
{
public:
	PlaneTerrainMeshGeneratorComponent();
	PlaneTerrainMeshGeneratorComponent(const PlaneTerrainMeshGeneratorComponent&) = default;
	PlaneTerrainMeshGeneratorComponent(PlaneTerrainMeshGeneratorComponent&&) = default;
	PlaneTerrainMeshGeneratorComponent& operator=(const PlaneTerrainMeshGeneratorComponent&) = default;
	PlaneTerrainMeshGeneratorComponent& operator=(PlaneTerrainMeshGeneratorComponent&&) = default;
	~PlaneTerrainMeshGeneratorComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] DirectX::SimpleMath::Vector2 GetPlaneSize() const { return DirectX::SimpleMath::Vector2(PlaneSize[0], PlaneSize[1]); }
	[[nodiscard]] int GetQuadSize() const { return QuadSize; }
	[[nodiscard]] float GetHeightmapVerticalScale() const { return HeightmapVerticalScale; }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Plane Generator"; }

private:
	virtual void GenerateMesh(int width, int height);

	bool UseHeightmap{ false };
	bool StaticHeightmap{ false };
	float HeightmapVerticalScale{ 50.0f };

	int PlaneSize[2]{ 1, 1 };
	int QuadSize{ 1 };

	TerrainConstantBuffer TerrainCBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> TerrainCBuffer;
};
