#pragma once

// TODO: Make this a generic base-class

class Shader
{
public:
	Shader();
	Shader(const Shader&) = default;
	Shader(Shader&&) = default;
	Shader& operator=(const Shader&) = default;
	Shader& operator=(Shader&&) = default;
	~Shader() = default;

	[[nodiscard]] ID3D11VertexShader* GetVertexShader() const { return VertexShader.Get(); }
	[[nodiscard]] ID3D11HullShader* GetHullShader() const { return HullShader.Get(); }
	[[nodiscard]] ID3D11DomainShader* GetDomainShader() const { return DomainShader.Get(); }
	[[nodiscard]] ID3D11PixelShader* GetPixelShader() const { return PixelShader.Get(); }
	[[nodiscard]] ID3D11InputLayout* GetInputLayout() const { return InputLayout.Get(); }
	[[nodiscard]] ID3D11SamplerState* GetSamplerState() const { return LinearSampler.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader> HullShader;
	Microsoft::WRL::ComPtr<ID3D11DomainShader> DomainShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> LinearSampler;

	D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[3]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT InLayoutNumElements = ARRAYSIZE(InputLayoutDesc);
};
