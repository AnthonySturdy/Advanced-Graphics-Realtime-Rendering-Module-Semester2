#pragma once
#include "Rendering/RenderPass.h"

class GameObject;

class RenderPassGeometry : public RenderPass
{
public:
	RenderPassGeometry(std::vector<GameObject*>& gameObjects);
	RenderPassGeometry(const RenderPassGeometry&) = default;
	RenderPassGeometry(RenderPassGeometry&&) = default;
	RenderPassGeometry& operator=(const RenderPassGeometry&) = delete;
	RenderPassGeometry& operator=(RenderPassGeometry&&) = delete;
	~RenderPassGeometry() override = default;

	void Initialise() override;
	void Render() override;
	void RenderGUI() override;

	[[nodiscard]] ID3D11ShaderResourceView* GetSRV() const { return RenderTargetSRV.Get(); }

private:
	std::vector<GameObject*>& GameObjects;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RenderStateFull;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> RenderStateWireframe;
	bool RenderWireframe{ false };

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderTargetSRV;
};
