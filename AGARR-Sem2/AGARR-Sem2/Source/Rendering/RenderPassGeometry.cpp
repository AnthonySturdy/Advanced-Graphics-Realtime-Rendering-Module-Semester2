#include "pch.h"
#include "Rendering/RenderPassGeometry.h"

#include "Game/GameObject.h"

RenderPassGeometry::RenderPassGeometry(std::vector<GameObject*>& gameObjects)
	: GameObjects(gameObjects) {}

void RenderPassGeometry::Initialise()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();
	const auto outputSize = DX::DeviceResources::Instance()->GetOutputSize();

	// Create render texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> rtvTex;
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = outputSize.right;
	texDesc.Height = outputSize.bottom;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	DX::ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, rtvTex.GetAddressOf()));
	DX::ThrowIfFailed(device->CreateRenderTargetView(rtvTex.Get(), nullptr, RenderTargetView.ReleaseAndGetAddressOf()));

	// Create depth stencil
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTex;
	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Width = outputSize.right;
	depthTexDesc.Height = outputSize.bottom;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	DX::ThrowIfFailed(device->CreateTexture2D(&depthTexDesc, nullptr, depthStencilTex.GetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	DX::ThrowIfFailed(device->CreateDepthStencilView(depthStencilTex.Get(), &depthStencilViewDesc, DepthStencilView.ReleaseAndGetAddressOf()));

	// Create render state
	D3D11_RASTERIZER_DESC renderStateDesc = {};
	renderStateDesc.CullMode = D3D11_CULL_BACK;
	renderStateDesc.FillMode = D3D11_FILL_SOLID;
	renderStateDesc.DepthClipEnable = true;
	DX::ThrowIfFailed(device->CreateRasterizerState(&renderStateDesc, RenderState.ReleaseAndGetAddressOf()));

	// Create SRV into RTV
	Microsoft::WRL::ComPtr<ID3D11Resource> geometryPassResource;
	RenderTargetView->GetResource(geometryPassResource.ReleaseAndGetAddressOf());
	DX::ThrowIfFailed(device->CreateShaderResourceView(geometryPassResource.Get(), nullptr, RenderTargetSRV.ReleaseAndGetAddressOf()));
}

void RenderPassGeometry::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();
	const auto outputSize = DX::DeviceResources::Instance()->GetOutputSize();

	static constexpr DirectX::SimpleMath::Color clearColour(1.0f, 0.0f, 0.0f, 1.0f);
	context->ClearRenderTargetView(RenderTargetView.Get(), &clearColour.x);
	context->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind resources
	context->OMSetRenderTargets(1, RenderTargetView.GetAddressOf(), DepthStencilView.Get());

	const CD3D11_VIEWPORT viewport(0.0f, 0.0f,
	                               outputSize.right, outputSize.bottom,
	                               0.0f, 1.0f);
	context->RSSetViewports(1, &viewport);

	context->RSSetState(RenderState.Get());

	// Render GameObjects
	for (const auto go : GameObjects)
		go->Render();

	// Unbind render targets
	ID3D11RenderTargetView* nullRtv = nullptr;
	ID3D11DepthStencilView* nullDsv = nullptr;
	context->OMSetRenderTargets(1, &nullRtv, nullDsv);
}
