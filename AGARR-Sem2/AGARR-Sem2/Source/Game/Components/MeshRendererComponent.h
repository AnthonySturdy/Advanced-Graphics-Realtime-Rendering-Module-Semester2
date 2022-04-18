#pragma once
#include "Game/GameObject.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"

struct PerObjectConstantBuffer
{
	DirectX::SimpleMath::Matrix World{ DirectX::SimpleMath::Matrix::Identity };
	float TessellationAmount{ 1.0f };
	float TesselationFalloff{ 7.5f };
	float padding[2]{};
};

class MeshRendererComponent : public Component
{
public:
	MeshRendererComponent();
	MeshRendererComponent(const MeshRendererComponent&) = default;
	MeshRendererComponent(MeshRendererComponent&&) = default;
	MeshRendererComponent& operator=(const MeshRendererComponent&) = default;
	MeshRendererComponent& operator=(MeshRendererComponent&&) = default;
	~MeshRendererComponent() override = default;

	void Update(float deltaTime) override;
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] Mesh* GetMesh() const { return MeshData.get(); }
	[[nodiscard]] Shader* GetShader() const { return MeshShader.get(); }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Mesh Renderer"; }

private:
	void CreateConstantBuffer();

	std::shared_ptr<Mesh> MeshData{ nullptr };
	std::shared_ptr<Shader> MeshShader{ nullptr };
	float TessellationAmount{ 1.0f };
	float TesselationFalloff{ 7.5f };

	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
};
