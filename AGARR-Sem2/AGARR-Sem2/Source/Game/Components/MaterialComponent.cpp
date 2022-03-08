#include "pch.h"
#include "MaterialComponent.h"

void MaterialComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	ID3D11ShaderResourceView* nsrv = nullptr;
	for (int i = 0; i < MaxTextures; ++i)
		if (i < Textures.size())
			context->PSSetShaderResources(i, 1, Textures[i].GetAddressOf());
		else
			context->PSSetShaderResources(i, 1, &nsrv);
}

void MaterialComponent::AddTexture(const std::wstring& texPath)
{
	if (Textures.size() == MaxTextures)
		return;

	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();


	Textures.push_back(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>());
	DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(device,
	                                                    texPath.c_str(),
	                                                    nullptr,
	                                                    Textures[Textures.size() - 1].ReleaseAndGetAddressOf()));
}
