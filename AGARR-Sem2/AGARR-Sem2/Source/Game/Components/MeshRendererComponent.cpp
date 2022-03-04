#include "pch.h"
#include "MeshRendererComponent.h"
#include "TransformComponent.h"

MeshRendererComponent::MeshRendererComponent()
{
	MeshData = std::make_shared<Mesh>();
	MeshShader = std::make_shared<Shader>();

	CreateConstantBuffer();
}

void MeshRendererComponent::Update(float deltaTime) { }

void MeshRendererComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	// Update and bind constant buffer
	static PerObjectConstantBuffer pocb = {};
	pocb.World = XMMatrixTranspose(Parent->GetComponent<TransformComponent>()->GetWorldMatrix());
	pocb.TessellationAmount = TessellationAmount;
	context->UpdateSubresource(ConstantBuffer.Get(), 0, nullptr, &pocb, 0, 0);

	context->VSSetConstantBuffers(1, 1, ConstantBuffer.GetAddressOf());
	context->HSSetConstantBuffers(1, 1, ConstantBuffer.GetAddressOf());
	context->DSSetConstantBuffers(1, 1, ConstantBuffer.GetAddressOf());

	// Bind shaders
	context->VSSetShader(MeshShader->GetVertexShader(), nullptr, 0);
	context->HSSetShader(MeshShader->GetHullShader(), nullptr, 0);
	context->DSSetShader(MeshShader->GetDomainShader(), nullptr, 0);
	context->PSSetShader(MeshShader->GetPixelShader(), nullptr, 0);

	// Bind input layout
	context->IASetInputLayout(MeshShader->GetInputLayout());

	// Bind sampler
	const auto sampler = MeshShader->GetSamplerState();
	context->PSSetSamplers(0, 1, &sampler);

	// Bind vertex and index buffers
	ID3D11Buffer* const vBuf = MeshData->GetVertexBuffer();
	static constexpr UINT stride = sizeof(Vertex);
	static constexpr UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);

	ID3D11Buffer* iBuf = MeshData->GetIndexBuffer();
	context->IASetIndexBuffer(iBuf, DXGI_FORMAT_R32_UINT, 0);

	// Draw object
	context->DrawIndexed(MeshData->GetNumIndices(), 0, 0);
}

void MeshRendererComponent::RenderGUI()
{
	// Model loading
	static char* path = new char[512]{};
	ImGui::InputText("##", path, 512, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("Choose Model"))
		ImGuiFileDialog::Instance()->OpenDialog("SelectObjModel", "Choose File", ".obj", ".");

	if (ImGuiFileDialog::Instance()->Display("SelectObjModel"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			const std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

			delete[] path;
			path = new char[512]{};
			for (int i = 0; i < filePath.size(); ++i)
				path[i] = filePath[i];

			MeshData->Initialise(std::wstring(filePath.begin(), filePath.end()));
		}
		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::SliderFloat("Tessellation Amount", &TessellationAmount, 1.0f, 32.0f);
}

void MeshRendererComponent::CreateConstantBuffer()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PerObjectConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, ConstantBuffer.ReleaseAndGetAddressOf()));
}
