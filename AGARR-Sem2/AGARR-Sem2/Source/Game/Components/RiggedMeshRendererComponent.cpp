#include "pch.h"
#include "RiggedMeshRendererComponent.h"
#include "MeshRendererComponent.h"
#include "TransformComponent.h"

RiggedMeshRendererComponent::RiggedMeshRendererComponent()
{
	MeshShader = std::make_shared<Shader>();

	CreateConstantBuffer();
}

void RiggedMeshRendererComponent::Update(float deltaTime)
{
	if (Animations.size() == 0)
		return;

	int animation = 0;

	ModelAnimation& anim = Animations[animation];

	Animations[animation].CurrAnimTime += deltaTime;
	if (anim.CurrAnimTime > anim.TotalAnimTime)
		anim.CurrAnimTime = 0.0f;

	const float currentFrame = anim.CurrAnimTime * anim.FrameRate;
	const int frame0 = floor(currentFrame);
	int frame1 = frame0 + 1;

	if (frame0 == anim.NumFrames - 1)
		frame1 = 0;

	const float interpolation = currentFrame - frame0;
	std::vector<Joint> interpolatedSkeleton{};

	for (int i = 0; i < anim.NumJoints; ++i)
	{
		Joint j;
		const Joint joint0 = anim.FrameSkeleton[frame0][i];
		const Joint joint1 = anim.FrameSkeleton[frame1][i];

		j.ParentID = joint0.ParentID;

		DirectX::SimpleMath::Quaternion joint0Orient = joint0.Orientation;
		DirectX::SimpleMath::Quaternion joint1Orient = joint1.Orientation;

		j.Position = DirectX::SimpleMath::Vector3::Lerp(joint0.Position, joint1.Position, interpolation);
		j.Orientation = DirectX::SimpleMath::Quaternion::Slerp(joint0Orient, joint1Orient, interpolation);

		interpolatedSkeleton.push_back(j);
	}

	for (int k = 0; k < NumSubsets; ++k)
	{
		for (int i = 0; i < Subsets[k].MeshData->GetVertices().size(); ++i)
		{
			Vertex v = Subsets[k].MeshData->GetVertices()[i];

			v.Pos = DirectX::SimpleMath::Vector3::Zero;
			v.Normal = DirectX::SimpleMath::Vector3::Zero;

			// Sum up joints and weights information to get vertex position and normal
			for (int j = 0; j < v.WeightCount; ++j)
			{
				Weight weight = Subsets[k].Weights[v.StartWeight + j];
				Joint joint = interpolatedSkeleton[weight.JointID];

				DirectX::SimpleMath::Quaternion jointOrientation = joint.Orientation;
				DirectX::SimpleMath::Vector3 weightPos = weight.Position;
				DirectX::SimpleMath::Quaternion jointOrientationConjugate;
				jointOrientation.Conjugate(jointOrientationConjugate);

				DirectX::SimpleMath::Vector3 rotatedPoint = XMQuaternionMultiply(XMQuaternionMultiply(jointOrientation, weightPos), jointOrientationConjugate);

				// Move the verices position from joint space
				v.Pos += (joint.Position + rotatedPoint) * weight.Bias;

				// Compute normals for this frames skeleton using the weight normals
				DirectX::SimpleMath::Vector3 weightNormal = weight.Normal;

				// Rotate normal
				rotatedPoint = XMQuaternionMultiply(XMQuaternionMultiply(jointOrientation, weightNormal), jointOrientationConjugate);

				// Add to vertices normal
				v.Normal -= rotatedPoint * weight.Bias;
			}

			Subsets[k].MeshData->GetVertices()[i].Pos = v.Pos;
			v.Normal.Normalize();
			// TODO: Fix normal rotation
			//Subsets[k].MeshData->GetVertices()[i].Normal = v.Normal;
		}

		Subsets[k].MeshData->RefreshVertexBuffer();
	}
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
		ImGuiFileDialog::Instance()->OpenDialog("SelectMd5Model", "Choose File", ".md5mesh", ".");

	if (ImGuiFileDialog::Instance()->Display("SelectMd5Model"))
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

	static char* path1 = new char[512]{};
	ImGui::InputText("##", path1, 512, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("Choose Anim"))
		ImGuiFileDialog::Instance()->OpenDialog("SelectMd5Anim", "Choose File", ".md5anim", ".");

	if (ImGuiFileDialog::Instance()->Display("SelectMd5Anim"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			const std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

			delete[] path1;
			path1 = new char[512]{};
			for (int i = 0; i < filePath.size(); ++i)
				path1[i] = filePath[i];

			LoadMD5Anim(std::wstring(filePath.begin(), filePath.end()));
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
	Subsets.clear();
	Joints.clear();
	Animations.clear();

	std::wifstream inFile(path);
	if (inFile.is_open())
	{
		std::wstring checkString;

		while (inFile)
		{
			inFile >> checkString;

			// Load header information
			if (checkString == L"MD5Version")
			{
				inFile >> checkString;
				if (checkString != L"10") throw std::invalid_argument("Only MD5 Version 10 is supported.");
			}
			else if (checkString == L"commandline")
				std::getline(inFile, checkString); // Skip rest of line
			else if (checkString == L"numJoints")
				inFile >> NumJoints;
			else if (checkString == L"numMeshes")
				inFile >> NumSubsets;
			else if (checkString == L"joints")
			{
				inFile >> checkString; // Skip '{'

				// Load joints
				for (int i = 0; i < NumJoints; ++i)
				{
					Joint joint;

					// Load joint name
					inFile >> joint.Name;
					joint.Name.erase(0, 1);
					while (!joint.Name.ends_with(L"\""))
					{
						inFile >> checkString;
						joint.Name += L" " + checkString;
					}
					joint.Name.erase(joint.Name.end() - 1);

					// Load joint parent id
					inFile >> joint.ParentID;

					inFile >> checkString; // Skip '('

					// Load joint position
					inFile >> joint.Position.x >> joint.Position.z >> joint.Position.y;

					inFile >> checkString >> checkString; // Skip ')' and '('

					// Load joint orientation and calculate w axis of quaternion
					inFile >> joint.Orientation.x >> joint.Orientation.z >> joint.Orientation.y;
					const float t = 1.0f - (joint.Orientation.x * joint.Orientation.x)
						- (joint.Orientation.y * joint.Orientation.y)
						- (joint.Orientation.z * joint.Orientation.z);
					if (t < 0.0f)
						joint.Orientation.w = 0.0f;
					else
						joint.Orientation.w = -sqrtf(t);

					std::getline(inFile, checkString); // Skip rest of line

					Joints.push_back(joint);
				}

				inFile >> checkString; // Skip '}'
			}
			else if (checkString == L"mesh")
			{
				// Load subset mesh
				ModelSubset subset;
				int numVerts = 0, numTris = 0, numWeights = 0;
				std::vector<Vertex> vertices;
				std::vector<UINT> indices;

				inFile >> checkString; // Skip '{'

				inFile >> checkString;
				while (checkString != L"}") // Loop until end of mesh
				{
					if (checkString == L"shader")
						std::getline(inFile, checkString); // Todo: Load texture with material component
					else if (checkString == L"numverts")
					{
						inFile >> numVerts;
						std::getline(inFile, checkString); // Skip rest of line

						for (int i = 0; i < numVerts; ++i)
						{
							Vertex v;
							inFile >> checkString >> checkString >> checkString; // Skip 'vert <counter> ('

							inFile >> v.TexCoord.x >> v.TexCoord.y;

							inFile >> checkString; // Skip ')'

							inFile >> v.StartWeight;
							inFile >> v.WeightCount;

							std::getline(inFile, checkString); // Skip rest of line

							vertices.push_back(v);
						}
					}
					else if (checkString == L"numtris")
					{
						inFile >> numTris;

						for (int i = 0; i < numTris; ++i)
						{
							int idx;
							inFile >> checkString >> checkString; // Skip 'tri <counter>'

							// Load indices
							inFile >> idx;
							indices.push_back(idx);

							inFile >> idx;
							indices.push_back(idx);

							inFile >> idx;
							indices.push_back(idx);

							std::getline(inFile, checkString); // Skip rest of line
						}
					}
					else if (checkString == L"numweights")
					{
						inFile >> numWeights;
						std::getline(inFile, checkString); // Skip rest of line

						for (int i = 0; i < numWeights; ++i)
						{
							Weight weight;

							inFile >> checkString >> checkString; // Skip 'weight <counter>'

							inFile >> weight.JointID;
							inFile >> weight.Bias;

							inFile >> checkString; // Skip '('

							inFile >> weight.Position.x >> weight.Position.z >> weight.Position.y;

							std::getline(inFile, checkString); // Skip rest of line

							subset.Weights.push_back(weight);
						}
					}
					else
						std::getline(inFile, checkString); // Skip unsupported lines

					inFile >> checkString;
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
				std::vector<DirectX::SimpleMath::Vector3> tempNormals;

				DirectX::SimpleMath::Vector3 unnormalized{ DirectX::SimpleMath::Vector3::Zero };
				DirectX::SimpleMath::Vector3 edge1{ DirectX::SimpleMath::Vector3::Zero };
				DirectX::SimpleMath::Vector3 edge2{ DirectX::SimpleMath::Vector3::Zero };

				//Compute face normals
				for (int i = 0; i < numTris; ++i)
				{
					//Get the vector describing edge of triangle (edge 0,2)
					edge1 = vertices[indices[(i * 3)]].Pos - vertices[indices[(i * 3) + 2]].Pos; //Create our first edge

					//Get the vector describing edge of triangle (edge 2,1)
					edge2 = vertices[indices[(i * 3) + 2]].Pos - vertices[indices[(i * 3) + 1]].Pos; //Create our second edge

					unnormalized = edge1.Cross(edge2);

					tempNormals.push_back(unnormalized);
				}

				DirectX::SimpleMath::Vector3 normalSum{ 0.0f, 0.0f, 0.0f };
				int facesUsing = 0;
				for (int i = 0; i < vertices.size(); ++i)
				{
					//Check which triangles use this vertex
					for (int j = 0; j < numTris; ++j)
					{
						if (indices[j * 3] == i ||
							indices[(j * 3) + 1] == i ||
							indices[(j * 3) + 2] == i)
						{
							normalSum += tempNormals[j];

							facesUsing++;
						}
					}

					normalSum = normalSum / facesUsing;
					normalSum.Normalize();

					vertices[i].Normal = -normalSum;

					normalSum = DirectX::SimpleMath::Vector3::Zero;
					facesUsing = 0;
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
		throw std::invalid_argument("Unable to open .md5mesh file!");
	}
}

void RiggedMeshRendererComponent::LoadMD5Anim(const std::wstring& path)
{
	std::wifstream inFile(path);
	if (inFile.is_open())
	{
		ModelAnimation anim;

		std::wstring checkString;

		while (inFile)
		{
			inFile >> checkString;

			// Load header information
			if (checkString == L"MD5Version")
			{
				inFile >> checkString;
				if (checkString != L"10") throw std::invalid_argument("Only MD5 Version 10 is supported.");
			}
			else if (checkString == L"commandline")
				std::getline(inFile, checkString); // Skip rest of line
			else if (checkString == L"numFrames")
				inFile >> anim.NumFrames;
			else if (checkString == L"numJoints")
				inFile >> anim.NumJoints;
			else if (checkString == L"frameRate")
				inFile >> anim.FrameRate;
			else if (checkString == L"numAnimatedComponents")
				inFile >> anim.NumAnimatedComponents;
			else if (checkString == L"hierarchy")
			{
				// Load animation joints
				inFile >> checkString;

				for (int i = 0; i < anim.NumJoints; ++i)
				{
					AnimJointInfo joint;

					inFile >> joint.Name;
					joint.Name.erase(0, 1);
					while (!joint.Name.ends_with(L"\""))
					{
						inFile >> checkString;
						joint.Name += L" " + checkString;
					}
					joint.Name.erase(joint.Name.end() - 1);

					inFile >> joint.ParentID;
					inFile >> joint.Flags;
					inFile >> joint.StartIndex;

					// Verify joint exists in mesh
					bool jointMatchFound = false;
					for (int j = 0; j < NumJoints; ++j)
						if (Joints[j].Name == joint.Name &&
							Joints[j].ParentID == joint.ParentID)
						{
							jointMatchFound = true;
							anim.JointInfo.push_back(joint);
						}
					if (!jointMatchFound)
						throw std::invalid_argument("Anim incompatible with loaded mesh");

					std::getline(inFile, checkString); // Skip rest of line
				}
			}
			else if (checkString == L"bounds")
			{
				inFile >> checkString;

				for (int i = 0; i < anim.NumFrames; ++i)
				{
					BoundingBox bb;

					inFile >> checkString; // Skip '('
					inFile >> bb.Min.x >> bb.Min.z >> bb.Min.y;
					inFile >> checkString >> checkString; // Skip ') ('
					inFile >> bb.Max.x >> bb.Max.z >> bb.Max.y;
					inFile >> checkString;

					anim.FrameBounds.push_back(bb);
				}
			}
			else if (checkString == L"baseframe")
			{
				inFile >> checkString;

				for (int i = 0; i < anim.NumJoints; ++i)
				{
					Joint bfj;

					inFile >> checkString; // Skip '('
					inFile >> bfj.Position.x >> bfj.Position.z >> bfj.Position.y;
					inFile >> checkString >> checkString; // Skip ') ('
					inFile >> bfj.Orientation.x >> bfj.Orientation.z >> bfj.Orientation.y;
					inFile >> checkString;

					anim.BaseFrameJoints.push_back(bfj);
				}
			}
			else if (checkString == L"frame")
			{
				FrameData frame;

				inFile >> frame.FrameID;

				inFile >> checkString; // Skip '{'

				for (int i = 0; i < anim.NumAnimatedComponents; ++i)
				{
					float tempData;
					inFile >> tempData;

					frame.Data.push_back(tempData);
				}

				anim.Frames.push_back(frame);

				// Build frame skeleton
				std::vector<Joint> skeleton;
				for (int i = 0; i < anim.JointInfo.size(); i++)
				{
					int k = 0; // Keep track of position in frameData array

					// Start the frames joint with the base frame's joint
					Joint frameJoint = anim.BaseFrameJoints[i];

					frameJoint.ParentID = anim.JointInfo[i].ParentID;

					if (anim.JointInfo[i].Flags & 1) // pos.x    ( 000001 )
						frameJoint.Position.x = frame.Data[anim.JointInfo[i].StartIndex + k++];

					if (anim.JointInfo[i].Flags & 2) // pos.y    ( 000010 )
						frameJoint.Position.z = frame.Data[anim.JointInfo[i].StartIndex + k++];

					if (anim.JointInfo[i].Flags & 4) // pos.z    ( 000100 )
						frameJoint.Position.y = frame.Data[anim.JointInfo[i].StartIndex + k++];

					if (anim.JointInfo[i].Flags & 8) // orientation.x    ( 001000 )
						frameJoint.Orientation.x = frame.Data[anim.JointInfo[i].StartIndex + k++];

					if (anim.JointInfo[i].Flags & 16) // orientation.y    ( 010000 )
						frameJoint.Orientation.z = frame.Data[anim.JointInfo[i].StartIndex + k++];

					if (anim.JointInfo[i].Flags & 32) // orientation.z    ( 100000 )
						frameJoint.Orientation.y = frame.Data[anim.JointInfo[i].StartIndex + k++];


					// Compute the quaternion's w
					const float t = 1.0f - (frameJoint.Orientation.x * frameJoint.Orientation.x)
						- (frameJoint.Orientation.y * frameJoint.Orientation.y)
						- (frameJoint.Orientation.z * frameJoint.Orientation.z);
					if (t < 0.0f)
					{
						frameJoint.Orientation.w = 0.0f;
					}
					else
					{
						frameJoint.Orientation.w = -sqrtf(t);
					}

					// Update skeleton joint position based on parent's orientation
					if (frameJoint.ParentID >= 0)
					{
						Joint parentJoint = skeleton[frameJoint.ParentID];

						DirectX::XMVECTOR parentJointOrientation = parentJoint.Orientation;
						DirectX::XMVECTOR tempJointPos = frameJoint.Position;
						DirectX::SimpleMath::Quaternion pjoc;
						parentJoint.Orientation.Conjugate(pjoc);
						DirectX::XMVECTOR parentOrientationConjugate = pjoc;

						// Calculate current joints position relative to its parents position
						DirectX::SimpleMath::Vector3 rotatedPos = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionMultiply(parentJointOrientation, tempJointPos), parentOrientationConjugate);

						// Translate the joint to model space by adding the parent joint's pos to it
						frameJoint.Position = rotatedPos + parentJoint.Position;

						// Currently the joint is oriented in its parent joints space, orient it in model space
						DirectX::SimpleMath::Quaternion tempJointOrient = frameJoint.Orientation;
						tempJointOrient = XMQuaternionMultiply(parentJointOrientation, tempJointOrient);

						tempJointOrient.Normalize();

						frameJoint.Orientation = tempJointOrient;
					}

					skeleton.push_back(frameJoint);
				}

				anim.FrameSkeleton.push_back(skeleton);

				inFile >> checkString; // Skip '}'
			}
		}

		anim.FrameTime = 1.0f / anim.FrameRate;
		anim.TotalAnimTime = anim.NumFrames * anim.FrameTime;

		Animations.push_back(anim);
	}
	else
	{
		throw std::invalid_argument("Unable to open .md5mesh file!");
	}
}
