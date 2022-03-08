#pragma once
#include "Game/GameObject.h"

class PlaneMeshGeneratorComponent : public Component
{
	struct TerrainConstantBuffer
	{
		BOOL ApplyHeightmap{ false };
		float HeightmapScale{ 0.15f };
		unsigned int NumTextures{ 0u };
		float padding{};
	};

public:
	PlaneMeshGeneratorComponent();
	PlaneMeshGeneratorComponent(const PlaneMeshGeneratorComponent&) = default;
	PlaneMeshGeneratorComponent(PlaneMeshGeneratorComponent&&) = default;
	PlaneMeshGeneratorComponent& operator=(const PlaneMeshGeneratorComponent&) = default;
	PlaneMeshGeneratorComponent& operator=(PlaneMeshGeneratorComponent&&) = default;
	~PlaneMeshGeneratorComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] bool IsHeightmapUsed() const { return UseHeightmap; }
	[[nodiscard]] unsigned int GetHeightmapSize() const { return HeightmapSize; }
	[[nodiscard]] float GetHeightmapVerticalScale() const { return HeightmapVerticalScale; }

	[[nodiscard]] DirectX::SimpleMath::Vector2 GetPlaneSize() const { return DirectX::SimpleMath::Vector2(PlaneSize[0], PlaneSize[1]); }

	[[nodiscard]] unsigned char SampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char SampleHeightmap(float normx, float normy) const;
	[[nodiscard]] unsigned char WrapSampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char WrapSampleHeightmap(float normx, float normy) const;

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Plane Mesh Generator"; }

private:
	void GeneratePlane(int width, int height, bool useHeightmap = false, float heightMapScale = 1.0f);

	void LoadHeightmap(const std::wstring& path);
	void GenerateFaultFormationHeightmap(unsigned int width = 256, unsigned int height = 256, unsigned int iterations = 600);
	void GenerateDiamondSquareHeightmap(unsigned int width = 513, unsigned int height = 513, unsigned int featureSize = 128);
	void SquareStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration);
	void DiamondStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration);
	void GeneratePerlinHeightmap(unsigned int width = 512, unsigned int height = 512, unsigned int iterations = 5);

	void CreateSrvFromHeightmapData();

	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(int x, int y) const;
	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(float normx, float normy) const;

	[[nodiscard]] float frand() const { return (rand() % 100000) / 100000.0f; }

	bool UseHeightmap{ false };
	bool StaticHeightmap{ false };
	unsigned int HeightmapSize = 0;
	float HeightmapVerticalScale{ 0.15f };

	int PlaneSize[2]{ 1, 1 };

	std::vector<unsigned char> Heightmap{};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> HeightmapSRV{};
	TerrainConstantBuffer TerrainCBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> TerrainCBuffer;
};
