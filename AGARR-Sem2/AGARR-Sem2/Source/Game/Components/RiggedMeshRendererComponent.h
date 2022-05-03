#pragma once
#include "Game/GameObject.h"

#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"

// Animation implemented referencing following guides:
// https://www.braynzarsoft.net/viewtutorial/q16390-27-loading-an-md5-model
// https://www.braynzarsoft.net/viewtutorial/q16390-28-skeletal-animation-based-on-the-md5-format

class RiggedMeshRendererComponent : public Component
{
	// MD5 Mesh
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
		DirectX::SimpleMath::Vector3 Normal{ DirectX::SimpleMath::Vector3::Zero };
	};

	struct ModelSubset
	{
		std::shared_ptr<Mesh> MeshData{ nullptr };
		std::vector<Weight> Weights{};
	};

	// MD5 Animation
	struct BoundingBox
	{
		DirectX::SimpleMath::Vector3 Min{ DirectX::SimpleMath::Vector3::Zero };
		DirectX::SimpleMath::Vector3 Max{ DirectX::SimpleMath::Vector3::Zero };
	};

	struct FrameData
	{
		int FrameID{ 0 };
		std::vector<float> Data{};
	};

	struct AnimJointInfo
	{
		std::wstring Name{};
		int ParentID{ -1 };

		int Flags{ 0 };
		int StartIndex{ 0 };
	};

	struct ModelAnimation
	{
		int NumFrames{ 0 };
		int NumJoints{ 0 };
		int FrameRate{ 0 };
		int NumAnimatedComponents{ 0 };

		float FrameTime{ 0.0f };
		float TotalAnimTime{ 0.0f };
		float CurrAnimTime{ 0.0f };

		std::vector<AnimJointInfo> JointInfo{};
		std::vector<BoundingBox> FrameBounds{};
		std::vector<Joint> BaseFrameJoints{};
		std::vector<FrameData> Frames{};
		std::vector<std::vector<Joint>> FrameSkeleton{};
	};

public:
	RiggedMeshRendererComponent();
	RiggedMeshRendererComponent(const RiggedMeshRendererComponent&) = default;
	RiggedMeshRendererComponent(RiggedMeshRendererComponent&&) = default;
	RiggedMeshRendererComponent& operator=(const RiggedMeshRendererComponent&) = default;
	RiggedMeshRendererComponent& operator=(RiggedMeshRendererComponent&&) = default;
	~RiggedMeshRendererComponent() override = default;

	void Update(float deltaTime) override;
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] Shader* GetShader() const { return MeshShader.get(); }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Rigged Mesh Renderer"; }

private:
	void CreateConstantBuffer();
	void LoadMD5Model(const std::wstring& path);
	void LoadMD5Anim(const std::wstring& path);

	int NumJoints{ 0 };
	int NumSubsets{ 0 };
	std::vector<ModelSubset> Subsets{};
	std::vector<Joint> Joints{};
	std::vector<ModelAnimation> Animations{};

	std::shared_ptr<Shader> MeshShader{ nullptr };

	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer;
};
