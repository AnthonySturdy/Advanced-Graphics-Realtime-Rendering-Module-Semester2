#pragma once
#include "Game/GameObject.h"

class MaterialComponent : public Component
{
public:
	MaterialComponent() = default;
	MaterialComponent(const MaterialComponent&) = default;
	MaterialComponent(MaterialComponent&&) = default;
	MaterialComponent& operator=(const MaterialComponent&) = default;
	MaterialComponent& operator=(MaterialComponent&&) = default;
	~MaterialComponent() override = default;

	void Update(float deltaTime) override {};
	void Render() override;
	void RenderGUI() override {};

	void AddTexture(const std::wstring& texPath);

	[[nodiscard]] int GetNumTextures() const { return Textures.size(); }

protected:
	[[nodiscard]] constexpr std::string GetComponentName() override { return "Material"; }

private:
	static inline constexpr int MaxTextures{ 10 };

	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Textures{};
};
