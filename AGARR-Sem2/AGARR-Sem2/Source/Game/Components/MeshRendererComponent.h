#pragma once
#include "Source/Game/GameObject.h"
#include "Source/Rendering/Mesh.h"
#include "Source/Rendering/Shader.h"

class MeshRendererComponent : public Component
{
public:
	MeshRendererComponent() = default;
	MeshRendererComponent(const MeshRendererComponent&) = default;
	MeshRendererComponent(MeshRendererComponent&&) = default;
	MeshRendererComponent& operator=(const MeshRendererComponent&) = default;
	MeshRendererComponent& operator=(MeshRendererComponent&&) = default;
	~MeshRendererComponent() override = default;

	void Update(float deltaTime) override;
	void Render() override;

private:
	std::shared_ptr<Mesh> MeshData{ nullptr };
	std::shared_ptr<Shader> MeshShader{ nullptr };
};
