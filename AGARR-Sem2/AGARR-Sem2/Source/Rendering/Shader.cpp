#include "pch.h"
#include "Shader.h"

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompileFromFile() below.
#endif

Shader::Shader()
{
	// TODO: Move this to Initialise() function, to allow for re-initialisation

	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	// Read and create Vertex shader
	ID3DBlob* vsBlob = nullptr;
	DX::ThrowIfFailed(D3DReadFileToBlob(L"Source/Rendering/Shaders/VertexShader.cso", &vsBlob));
	DX::ThrowIfFailed(device->CreateVertexShader(vsBlob->GetBufferPointer(),
	                                             vsBlob->GetBufferSize(),
	                                             nullptr,
	                                             VertexShader.ReleaseAndGetAddressOf()));

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

	// Release blobs
	vsBlob->Release();
	psBlob->Release();
}
