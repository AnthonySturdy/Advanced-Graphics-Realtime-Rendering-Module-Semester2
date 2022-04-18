#pragma once
#include "Game/GameObject.h"

#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"

class RiggedMeshRendererComponent : public Component
{
	struct Joint
	{
		std::wstring Name{};
		int ParentID{ -1 };

		DirectX::SimpleMath::Vector3 Position{ DirectX::SimpleMath::Vector3::Zero };
		DirectX::SimpleMath::Quaternion Orientation{ DirectX::SimpleMath::Quaternion::Identity };
	};

	struct Weight
	{
		int JointID{ -1 };
		float Bias{ 0.0f };
		DirectX::SimpleMath::Vector3 Position{ DirectX::SimpleMath::Vector3::Zero };
	};

	struct ModelSubset
	{
		int TexArrayIndex{ 0 };

		std::shared_ptr<Mesh> MeshData{ nullptr };
		std::vector<Weight> Weights{};

		//std::vector<DirectX::SimpleMath::Vector3> Positions{};
	};

public:
	RiggedMeshRendererComponent();
	RiggedMeshRendererComponent(const RiggedMeshRendererComponent&) = default;
	RiggedMeshRendererComponent(RiggedMeshRendererComponent&&) = default;
	RiggedMeshRendererComponent& operator=(const RiggedMeshRendererComponent&) = default;
	RiggedMeshRendererComponent& operator=(RiggedMeshRendererComponent&&) = default;
	~RiggedMeshRendererComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] Shader* GetShader() const { return MeshShader.get(); }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Rigged Mesh Renderer"; }

private:
	void CreateConstantBuffer();
	void LoadMD5Model(const std::wstring& path);

	int NumJoints{ 0 };
	int NumSubsets{ 0 };
	std::vector<ModelSubset> Subsets{};
	std::vector<Joint> Joints{};

	std::shared_ptr<Shader> MeshShader{ nullptr };

	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
};
