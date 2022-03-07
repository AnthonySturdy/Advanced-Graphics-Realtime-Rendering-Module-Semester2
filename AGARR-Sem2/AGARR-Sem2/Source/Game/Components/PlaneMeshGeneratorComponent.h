#pragma once
#include "Game/GameObject.h"

class PlaneMeshGeneratorComponent : public Component
{
	struct TerrainConstantBuffer
	{
		BOOL ApplyHeightmap{ false };
		float HeightmapScale{ 0.15f };
		float padding[2] = {};
	};

public:
	PlaneMeshGeneratorComponent();
	PlaneMeshGeneratorComponent(const PlaneMeshGeneratorComponent&) = default;
	PlaneMeshGeneratorComponent(PlaneMeshGeneratorComponent&&) = default;
	PlaneMeshGeneratorComponent& operator=(const PlaneMeshGeneratorComponent&) = default;
	PlaneMeshGeneratorComponent& operator=(PlaneMeshGeneratorComponent&&) = default;
	~PlaneMeshGeneratorComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override {};
	void RenderGUI() override;

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Plane Mesh Generator"; }

private:
	void GeneratePlane(int width, int height, bool useHeightmap = false, float heightMapScale = 1.0f);

	void LoadHeightmap(const std::wstring& path);
	void GenerateFaultFormationHeightmap(unsigned int width = 256, unsigned int height = 256, unsigned int iterations = 600);
	void GenerateDiamondSquareHeightmap(unsigned int width = 513, unsigned int height = 513, unsigned int featureSize = 128);
	void SquareStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration);
	void DiamondStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration);

	void CreateSrvFromHeightmapData();

	[[nodiscard]] unsigned char SampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char SampleHeightmap(float normx, float normy) const;
	[[nodiscard]] unsigned char WrapSampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char WrapSampleHeightmap(float normx, float normy) const;

	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(int x, int y) const;
	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(float normx, float normy) const;

	[[nodiscard]] float frand() const { return (rand() % 100000) / 100000.0f; }

	bool UseHeightmap{ false };
	bool StaticHeightmap{ false };
	unsigned int HeightmapSize = 0;
	float HeightmapVerticalScale{ 0.15f };
	std::vector<unsigned char> Heightmap{};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> HeightmapSRV{};
	TerrainConstantBuffer TerrainCBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> TerrainCBuffer;
};
