#include "pch.h"
#include "RiggedMeshRendererComponent.h"
#include "MeshRendererComponent.h"
#include "TransformComponent.h"

RiggedMeshRendererComponent::RiggedMeshRendererComponent()
{
	MeshShader = std::make_shared<Shader>();

	CreateConstantBuffer();
}

void RiggedMeshRendererComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	// Update and bind constant buffer
	static PerObjectConstantBuffer pocb = {};
	pocb.World = XMMatrixTranspose(Parent->GetComponent<TransformComponent>()->GetWorldMatrix());
	pocb.TessellationAmount = 1.0f;
	pocb.TesselationFalloff = 1.0f;
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
	context->DSSetSamplers(0, 1, &sampler);
	context->PSSetSamplers(0, 1, &sampler);

	// Bind vertex and index buffers
	for (const auto& subset : Subsets)
	{
		ID3D11Buffer* const vBuf = subset.MeshData->GetVertexBuffer();
		static constexpr UINT stride = sizeof(Vertex);
		static constexpr UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &vBuf, &stride, &offset);

		ID3D11Buffer* iBuf = subset.MeshData->GetIndexBuffer();
		context->IASetIndexBuffer(iBuf, DXGI_FORMAT_R32_UINT, 0);

		// Draw object
		context->DrawIndexed(subset.MeshData->GetNumIndices(), 0, 0);
	}
}

void RiggedMeshRendererComponent::RenderGUI()
{
	// Model loading
	static char* path = new char[512]{};
	ImGui::InputText("##", path, 512, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("Choose Model"))
		ImGuiFileDialog::Instance()->OpenDialog("SelectObjModel", "Choose File", ".md5mesh", ".");

	if (ImGuiFileDialog::Instance()->Display("SelectObjModel"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			const std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

			delete[] path;
			path = new char[512]{};
			for (int i = 0; i < filePath.size(); ++i)
				path[i] = filePath[i];

			LoadMD5Model(std::wstring(filePath.begin(), filePath.end()));
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void RiggedMeshRendererComponent::CreateConstantBuffer()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PerObjectConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, ConstantBuffer.ReleaseAndGetAddressOf()));
}

void RiggedMeshRendererComponent::LoadMD5Model(const std::wstring& path)
{
	std::wifstream fileIn(path);

	std::wstring checkString;

	if (fileIn.is_open())
	{
		while (fileIn)
		{
			fileIn >> checkString;

			// Load header information
			if (checkString == L"MD5Version")
			{
				fileIn >> checkString;
				if (checkString != L"10") throw std::invalid_argument("Only MD5 Version 10 is supported.");
			}
			else if (checkString == L"commandline")
				std::getline(fileIn, checkString); // Skip rest of line
			else if (checkString == L"numJoints")
				fileIn >> NumJoints;
			else if (checkString == L"numMeshes")
				fileIn >> NumSubsets;
			else if (checkString == L"joints")
			{
				fileIn >> checkString; // Skip '{'

				// Load joints
				for (int i = 0; i < NumJoints; ++i)
				{
					Joint joint;

					// Load joint name
					fileIn >> joint.Name;
					joint.Name.erase(0, 1);
					while (!joint.Name.ends_with(L"\""))
					{
						fileIn >> checkString;
						joint.Name += L" " + checkString;
					}
					joint.Name.erase(joint.Name.end() - 1);

					// Load joint parent id
					fileIn >> joint.ParentID;

					fileIn >> checkString; // Skip '('

					// Load joint position
					fileIn >> joint.Position.x >> joint.Position.z >> joint.Position.y;

					fileIn >> checkString >> checkString; // Skip ')' and '('

					// Load joint orientation and calculate w axis of quaternion
					fileIn >> joint.Orientation.x >> joint.Orientation.z >> joint.Orientation.y;
					const float t = 1.0f - (joint.Orientation.x * joint.Orientation.x)
						- (joint.Orientation.y * joint.Orientation.y)
						- (joint.Orientation.z * joint.Orientation.z);
					if (t < 0.0f)
						joint.Orientation.w = 0.0f;
					else
						joint.Orientation.w = -sqrtf(t);

					std::getline(fileIn, checkString); // Skip rest of line

					Joints.push_back(joint);
				}

				fileIn >> checkString; // Skip '}'
			}
			else if (checkString == L"mesh")
			{
				// Load subset mesh
				ModelSubset subset;
				int numVerts = 0, numTris = 0, numWeights = 0;
				std::vector<Vertex> vertices;
				std::vector<UINT> indices;

				fileIn >> checkString; // Skip '{'

				fileIn >> checkString;
				while (checkString != L"}") // Loop until end of mesh
				{
					if (checkString == L"shader")
						std::getline(fileIn, checkString); // Todo: Load texture with material component
					else if (checkString == L"numverts")
					{
						fileIn >> numVerts;
						std::getline(fileIn, checkString); // Skip rest of line

						for (int i = 0; i < numVerts; ++i)
						{
							Vertex v;
							fileIn >> checkString >> checkString >> checkString; // Skip 'vert <counter> ('

							fileIn >> v.TexCoord.x >> v.TexCoord.y;

							fileIn >> checkString; // Skip ')'

							fileIn >> v.StartWeight;
							fileIn >> v.WeightCount;

							std::getline(fileIn, checkString); // Skip rest of line

							vertices.push_back(v);
						}
					}
					else if (checkString == L"numtris")
					{
						fileIn >> numTris;

						for (int i = 0; i < numTris; ++i)
						{
							int idx;
							fileIn >> checkString >> checkString; // Skip 'tri <counter>'

							// Load indices
							fileIn >> idx;
							indices.push_back(idx);

							fileIn >> idx;
							indices.push_back(idx);

							fileIn >> idx;
							indices.push_back(idx);

							std::getline(fileIn, checkString); // Skip rest of line
						}
					}
					else if (checkString == L"numweights")
					{
						fileIn >> numWeights;
						std::getline(fileIn, checkString); // Skip rest of line

						for (int i = 0; i < numWeights; ++i)
						{
							Weight weight;

							fileIn >> checkString >> checkString; // Skip 'weight <counter>'

							fileIn >> weight.JointID;
							fileIn >> weight.Bias;

							fileIn >> checkString; // Skip '('

							fileIn >> weight.Position.x >> weight.Position.z >> weight.Position.y;

							std::getline(fileIn, checkString); // Skip rest of line

							subset.Weights.push_back(weight);
						}
					}
					else
						std::getline(fileIn, checkString); // Skip unsupported lines

					fileIn >> checkString;
				}

				// Calculate vertex position after all data loaded
				for (auto& vertex : vertices)
				{
					vertex.Pos = DirectX::SimpleMath::Vector3::Zero;

					// Sum joints and weights to calculate vertex position
					for (int i = 0; i < vertex.WeightCount; ++i)
					{
						const Weight w = subset.Weights[vertex.StartWeight + i];
						const Joint j = Joints[w.JointID];

						DirectX::SimpleMath::Quaternion conjugateOrientation = j.Orientation;
						conjugateOrientation.Conjugate();

						DirectX::XMVECTOR o = j.Orientation;
						DirectX::XMVECTOR co = conjugateOrientation;
						DirectX::XMVECTOR p = DirectX::SimpleMath::Vector4(w.Position.x, w.Position.y, w.Position.z, 0.0f);

						// Calculate vertex position in joint space and rotate
						DirectX::SimpleMath::Vector3 rotatedPoint = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(o, p), co);

						// Move vertices from joint space to the joint position in world space
						vertex.Pos += (j.Position + rotatedPoint) * w.Bias;
					}
				}

				// Calculate surface normals
				for (int i = 0; i < numTris; ++i)
				{
					// Calculate normal
					auto& p1 = vertices[indices[numTris]];
					auto& p2 = vertices[indices[numTris + 1]];
					auto& p3 = vertices[indices[numTris + 2]];

					const auto v1 = p2.Pos - p1.Pos;
					const auto v2 = p3.Pos - p1.Pos;
					DirectX::SimpleMath::Vector3 norm = v1.Cross(v2);
					norm.Normalize();

					p1.Normal = norm;
					p2.Normal = norm;
					p3.Normal = norm;
				}

				// Initialise sub-mesh
				subset.MeshData = std::make_shared<Mesh>();
				subset.MeshData->Initialise(vertices, indices);
				Subsets.push_back(subset);
			}
		}
	}
	else
	{
		throw std::invalid_argument("Unable to load .md5mesh file!");
	}
}
