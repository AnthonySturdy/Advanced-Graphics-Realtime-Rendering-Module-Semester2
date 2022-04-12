#pragma once
#include "Game/GameObject.h"

class HeightmapGeneratorComponent : public Component
{
public:
	HeightmapGeneratorComponent() = default;
	HeightmapGeneratorComponent(const HeightmapGeneratorComponent&) = default;
	HeightmapGeneratorComponent(HeightmapGeneratorComponent&&) = default;
	HeightmapGeneratorComponent& operator=(const HeightmapGeneratorComponent&) = default;
	HeightmapGeneratorComponent& operator=(HeightmapGeneratorComponent&&) = default;
	~HeightmapGeneratorComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override {};
	void RenderGUI() override;

	[[nodiscard]] ID3D11ShaderResourceView* const* GetHeightmapSRV() const { return HeightmapSRV.GetAddressOf(); }
	[[nodiscard]] unsigned int GetHeightmapSize() const { return HeightmapSize; }

	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(int x, int y) const;
	[[nodiscard]] DirectX::SimpleMath::Vector3 CalculateNormalAt(float normx, float normy) const;

	[[nodiscard]] unsigned char SampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char SampleHeightmap(float normx, float normy) const;
	[[nodiscard]] unsigned char WrapSampleHeightmap(int x, int y) const;
	[[nodiscard]] unsigned char WrapSampleHeightmap(float normx, float normy) const;

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Heightmap Generator"; }

private:
	void LoadHeightmapFromFile(const std::wstring& path);
	void GenerateFaultFormationHeightmap(unsigned int width = 512, unsigned int height = 512, unsigned int iterations = 600);
	void GenerateDiamondSquareHeightmap(unsigned int width = 513, unsigned int height = 513, unsigned int featureSize = 128);
	void SquareStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration);
	void DiamondStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration);
	void GeneratePerlinHeightmap(unsigned int width = 512, unsigned int height = 512, unsigned int iterations = 5);

	void CreateSrvFromHeightmapData();

	[[nodiscard]] float frand() const { return (rand() % 100000) / 100000.0f; }

	unsigned int HeightmapSize = 0;

	std::vector<unsigned char> Heightmap{};
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> HeightmapSRV{};
};
