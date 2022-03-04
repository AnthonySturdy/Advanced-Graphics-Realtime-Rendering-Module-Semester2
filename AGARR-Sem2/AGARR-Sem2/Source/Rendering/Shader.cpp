#include "pch.h"
#include "Shader.h"

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompileFromFile() below.
#endif

Shader::Shader()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	// Read and create Vertex shader
	ID3DBlob* vsBlob = nullptr;
	DX::ThrowIfFailed(D3DReadFileToBlob(L"Source/Rendering/Shaders/VertexShader.cso", &vsBlob));
	DX::ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(),
	                                             vsBlob->GetBufferSize(),
	                                             nullptr,
	                                             VertexShader.ReleaseAndGetAddressOf()));

	// Read and create Hull shader
	ID3DBlob* hsBlob = nullptr;
	DX::ThrowIfFailed(D3DReadFileToBlob(L"Source/Rendering/Shaders/HullShader.cso", &hsBlob));
	DX::ThrowIfFailed(device->CreateHullShader(hsBlob->GetBufferPointer(),
	                                           hsBlob->GetBufferSize(),
	                                           nullptr,
	                                           HullShader.ReleaseAndGetAddressOf()));

	// Read and create Domain shader
	ID3DBlob* dsBlob = nullptr;
	DX::ThrowIfFailed(D3DReadFileToBlob(L"Source/Rendering/Shaders/DomainShader.cso", &dsBlob));
	DX::ThrowIfFailed(device->CreateDomainShader(dsBlob->GetBufferPointer(),
	                                             dsBlob->GetBufferSize(),
	                                             nullptr,
	                                             DomainShader.ReleaseAndGetAddressOf()));

	// Read and create Pixel shader
	ID3DBlob* psBlob = nullptr;
	DX::ThrowIfFailed(D3DReadFileToBlob(L"Source/Rendering/Shaders/PixelShader.cso", &psBlob));
	DX::ThrowIfFailed(device->CreatePixelShader(psBlob->GetBufferPointer(),
	                                            psBlob->GetBufferSize(),
	                                            nullptr,
	                                            PixelShader.ReleaseAndGetAddressOf()));

	// Create the input layout
	DX::ThrowIfFailed(device->CreateInputLayout(InputLayoutDesc,
	                                            InLayoutNumElements,
	                                            vsBlob->GetBufferPointer(),
	                                            vsBlob->GetBufferSize(),
	                                            InputLayout.ReleaseAndGetAddressOf()));

	// Create sampler
	D3D11_SAMPLER_DESC sampDesc{};
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc, LinearSampler.ReleaseAndGetAddressOf()));

	// Release blobs
	vsBlob->Release();
	psBlob->Release();
}
