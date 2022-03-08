#include "pch.h"
#include "PlaneMeshGeneratorComponent.h"

#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "Rendering/Mesh.h"
#include "Utility/PerlinNoise.h"

PlaneMeshGeneratorComponent::PlaneMeshGeneratorComponent()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TerrainConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DX::ThrowIfFailed(device->CreateBuffer(&bd, nullptr, TerrainCBuffer.ReleaseAndGetAddressOf()));
}

void PlaneMeshGeneratorComponent::Render()
{
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();

	TerrainCBufferData.NumTextures = Parent->GetComponent<MaterialComponent>()->GetNumTextures();

	if (!UseHeightmap)
	{
		ID3D11ShaderResourceView* nsrv = nullptr;
		context->DSSetShaderResources(0, 1, &nsrv);

		TerrainCBufferData.ApplyHeightmap = false;
		TerrainCBufferData.HeightmapScale = 0.0f;
		context->UpdateSubresource(TerrainCBuffer.Get(), 0, nullptr, &TerrainCBufferData, 0, 0);
		context->DSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
		context->PSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
		return;
	}

	// Update shader resources
	context->DSSetShaderResources(0, 1, HeightmapSRV.GetAddressOf());

	TerrainCBufferData.ApplyHeightmap = !StaticHeightmap;
	TerrainCBufferData.HeightmapScale = HeightmapVerticalScale;
	context->UpdateSubresource(TerrainCBuffer.Get(), 0, nullptr, &TerrainCBufferData, 0, 0);
	context->DSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
	context->PSSetConstantBuffers(2, 1, TerrainCBuffer.GetAddressOf());
}

void PlaneMeshGeneratorComponent::RenderGUI()
{
	// Heightmap Generation/Selection
	const char* heightmapSourceListItems[] = { ".RAW File", "Fault Formation", "Diamond Square", "Perlin Noise" };
	static int heightMapSourceSelection = 0;
	ImGui::Combo("Heightmap Source", &heightMapSourceSelection, heightmapSourceListItems, 4);

	ImGui::Image(HeightmapSRV.Get(), ImVec2(20, 20)); // Heightmap preview and tooltip
	if (ImGui::IsItemHovered() && HeightmapSize != 0)
	{
		ImGui::BeginTooltip();
		ImGui::Image(HeightmapSRV.Get(), ImVec2(200, 200));
		ImGui::EndTooltip();
	}
	ImGui::SameLine();
	static char* path = new char[512]{}; // Heightmap file selection
	switch (heightMapSourceSelection)
	{
	default:
	case 0:
		ImGui::InputTextWithHint("##", "Select .RAW Heightmap File", path, 512, ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		if (ImGui::Button("..."))
			ImGuiFileDialog::Instance()->OpenDialog("SelectRawHeightmap", "Choose File", ".raw", ".");
		if (ImGuiFileDialog::Instance()->Display("SelectRawHeightmap"))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				const std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();

				delete[] path;
				path = new char[512]{};
				for (int i = 0; i < filePath.size(); ++i)
					path[i] = filePath[i];

				LoadHeightmap(std::wstring(filePath.begin(), filePath.end()));
			}
			ImGuiFileDialog::Instance()->Close();
		}
		break;
	case 1:
		if (ImGui::Button("Generate Heightmap"))
			GenerateFaultFormationHeightmap();
		break;
	case 2:
		if (ImGui::Button("Generate Heightmap"))
		{
			GenerateDiamondSquareHeightmap();
		}
		break;
	case 3:
		if (ImGui::Button("Generate Heightmap"))
		{
			GeneratePerlinHeightmap();
		}
		break;
	}

	// Heightmap controls
	if (HeightmapSize != 0)
		ImGui::Checkbox("Use Heightmap", &UseHeightmap);
	else
		UseHeightmap = false;

	if (UseHeightmap)
	{
		ImGui::Checkbox("Static", &StaticHeightmap);
		ImGui::DragFloat("Heightmap Scale", &HeightmapVerticalScale, 0.01f, 0.01f, 500.0f);
	}

	// Plane Size
	ImGui::DragInt2("Plane Size", &PlaneSize[0], 1, 1, UseHeightmap ? HeightmapSize : INT16_MAX);

	// Plane generation
	if (ImGui::Button("Generate Plane"))
	{
		GeneratePlane(PlaneSize[0], PlaneSize[1], UseHeightmap, HeightmapVerticalScale);
	}
}

void PlaneMeshGeneratorComponent::GeneratePlane(int width, int height, bool useHeightmap, float heightMapScale)
{
	const auto meshRenderer = Parent->GetComponent<MeshRendererComponent>();
	if (!meshRenderer)
		return;

	++width;
	++height;

	std::vector<Vertex> newVerts;
	std::vector<UINT> newIndices;
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			const float normX = static_cast<float>(x) / width;
			const float normY = static_cast<float>(y) / height;

			float heightMapSample = 0.0f;
			if (useHeightmap)
				heightMapSample = static_cast<float>(SampleHeightmap(normX, normY)) / 255.0f;

			// Generate vertices
			Vertex vert = {
				DirectX::SimpleMath::Vector3(x, StaticHeightmap ? heightMapSample * HeightmapVerticalScale : 0, y),
				CalculateNormalAt(normX, normY),
				DirectX::SimpleMath::Vector2(normX, normY)
			};
			newVerts.push_back(vert);

			// Generate indices
			if (x == width - 1 || y == height - 1)
				continue;
			//Triangle 1
			newIndices.push_back(y * width + x);
			newIndices.push_back((y + 1) * width + x);
			newIndices.push_back(y * width + (x + 1));

			//Triangle 2
			newIndices.push_back(y * width + (x + 1));
			newIndices.push_back((y + 1) * width + x);
			newIndices.push_back((y + 1) * width + (x + 1));
		}
	}

	Mesh* mesh = meshRenderer->GetMesh();
	mesh->Initialise(newVerts, newIndices);
}

void PlaneMeshGeneratorComponent::LoadHeightmap(const std::wstring& path)
{
	if (std::ifstream inFile(path.c_str(), std::ios_base::binary); inFile)
	{
		Heightmap = std::vector<unsigned char>(std::istreambuf_iterator<char>(inFile),
		                                       std::istreambuf_iterator<char>());

		HeightmapSize = std::sqrt(Heightmap.size());

		inFile.close();
	}

	CreateSrvFromHeightmapData();
}

void PlaneMeshGeneratorComponent::GenerateFaultFormationHeightmap(unsigned int width, unsigned int height, unsigned int iterations)
{
	std::vector<float> newHeightmap(width * height);

	for (unsigned int i = 0; i < iterations; ++i)
	{
		const DirectX::SimpleMath::Vector2 lineA(rand() % width, rand() % height);
		const DirectX::SimpleMath::Vector2 lineB(rand() % width, rand() % height);

		for (unsigned int x = 0; x < width; ++x)
		{
			for (unsigned int y = 0; y < height; ++y)
			{
				const unsigned int index = y * width + x;
				if (index >= newHeightmap.size())
					return;

				const DirectX::SimpleMath::Vector2 point(x, y);

				const float normIteration = (1.0f - (i / static_cast<float>(iterations)));
				const bool pointIncremented = (lineB.x - lineA.x) * (point.y - lineA.y) > (lineB.y - lineA.y) * (point.x - lineA.x);
				static constexpr float heightmapIntensity = 200.0f;
				const float valueChangeAmount = iterations / heightmapIntensity * normIteration;

				newHeightmap[index] += pointIncremented ? valueChangeAmount : -valueChangeAmount;
				newHeightmap[index] = std::clamp(newHeightmap[index], 0.0f, 255.0f);
			}
		}
	}

	Heightmap = std::vector<unsigned char>(newHeightmap.begin(), newHeightmap.end());
	HeightmapSize = width;
	CreateSrvFromHeightmapData();
}

void PlaneMeshGeneratorComponent::GenerateDiamondSquareHeightmap(unsigned int width, unsigned int height, unsigned int featureSize)
{
	// Reference: http://bluh.org/code-the-diamond-square-algorithm/

	featureSize = std::min(featureSize, height);

	std::vector<float> newHeightmap(width * height);

	// Create initial seed values
	for (int y = 0; y < height; y += featureSize)
		for (int x = 0; x < width; x += featureSize)
			newHeightmap[y * width + x] = frand();

	// SD Algorithm
	float i = 5.0f;
	while (featureSize > 1)
	{
		const unsigned int half = featureSize / 2;

		// Square
		for (int y = half; y < height; y += featureSize)
			for (int x = half; x < width; x += featureSize)
				SquareStage(newHeightmap, x % width, y % height, half, width, i);

		// Diamond
		int col = 0;
		for (int x = 0; x < width; x += half)
		{
			++col;

			//If this is an odd column.
			if (col % 2 == 1)
				for (int y = half; y < height; y += featureSize)
					DiamondStage(newHeightmap, x % width, y % height, half, width, i);
			else
				for (int y = 0; y < height; y += featureSize)
					DiamondStage(newHeightmap, x % width, y % height, half, width, i);
		}

		featureSize /= 2;
		i += 2.5f;
	}

	// Normalise heightmap
	float highest = 0.0f;
	for (const auto v : newHeightmap)
		highest = std::max(v, highest);

	for (auto& v : newHeightmap)
	{
		v /= highest;
		v *= 255.0f;
	}

	Heightmap = std::vector<unsigned char>(newHeightmap.begin(), newHeightmap.end());
	HeightmapSize = width;
	CreateSrvFromHeightmapData();
}

void PlaneMeshGeneratorComponent::SquareStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration)
{
	int count = 0;
	float avg = 0.0f;

	if (x - reach >= 0 && y - reach >= 0)
	{
		avg += hm[(y - reach) * size + (x - reach)];
		count++;
	}
	if (x - reach >= 0 && y + reach < size)
	{
		avg += hm[(y + reach) * size + (x - reach)];
		count++;
	}
	if (x + reach < size && y - reach >= 0)
	{
		avg += hm[(y - reach) * size + (x + reach)];
		count++;
	}
	if (x + reach < size && y + reach < size)
	{
		avg += hm[(y + reach) * size + (x + reach)];
		count++;
	}

	avg += frand() / iteration;
	avg /= count;

	hm[y * size + x] = avg;
}

void PlaneMeshGeneratorComponent::DiamondStage(std::vector<float>& hm, int x, int y, int reach, int size, float iteration)
{
	int count = 0;
	float avg = 0.0f;

	if (x - reach >= 0)
	{
		avg += hm[y * size + (x - reach)];
		count++;
	}
	if (x + reach < size)
	{
		avg += hm[y * size + (x + reach)];
		count++;
	}
	if (y - reach >= 0)
	{
		avg += hm[(y - reach) * size + x];
		count++;
	}
	if (y + reach < size)
	{
		avg += hm[(y + reach) * size + x];
		count++;
	}

	avg += frand() / iteration;
	avg /= count;

	hm[y * size + x] = avg;
}

void PlaneMeshGeneratorComponent::GeneratePerlinHeightmap(unsigned width, unsigned height, unsigned iterations)
{
	std::vector<float> newHeightmap(width * height);

	PerlinNoise::seed(rand());

	for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y)
			for (int i = 0; i < iterations; ++i)
			{
				const double sampleScale = std::pow(2.0, i + 4.0);
				const double sampleStrength = (iterations - i) * 5.0f;
				newHeightmap[y * width + x] += PerlinNoise::noise(x / sampleScale, y / sampleScale, 0) / (i == iterations - 1 ? 1 : sampleStrength);
			}

	// Normalise heightmap
	float highest = 0.0f;
	for (const auto v : newHeightmap)
		highest = std::max(v, highest);

	for (auto& v : newHeightmap)
	{
		v /= highest;
		v *= 255.0f;
	}

	Heightmap = std::vector<unsigned char>(newHeightmap.begin(), newHeightmap.end());
	HeightmapSize = width;
	CreateSrvFromHeightmapData();
}

void PlaneMeshGeneratorComponent::CreateSrvFromHeightmapData()
{
	if (Heightmap.size() == 0)
		return;

	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	const D3D11_SUBRESOURCE_DATA initData = { Heightmap.data(), HeightmapSize, 0 };

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = desc.Height = HeightmapSize;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	DX::ThrowIfFailed(device->CreateTexture2D(&desc,
	                                          &initData,
	                                          tex.ReleaseAndGetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_R8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	DX::ThrowIfFailed(device->CreateShaderResourceView(tex.Get(),
	                                                   &SRVDesc,
	                                                   HeightmapSRV.ReleaseAndGetAddressOf()));
}

unsigned char PlaneMeshGeneratorComponent::SampleHeightmap(int x, int y) const
{
	if (HeightmapSize == 0)
		return 0;

	if (x < 0 || y < 0 ||
		x >= HeightmapSize || y >= HeightmapSize)
		return 0;

	const int index = y * HeightmapSize + x;
	if (index >= Heightmap.size())
		throw std::out_of_range("Current values somehow passed checks but is still out of range.");

	return Heightmap[index];
}

unsigned char PlaneMeshGeneratorComponent::SampleHeightmap(float normx, float normy) const
{
	const float x = std::clamp(normx, 0.0f, 1.0f);
	const float y = std::clamp(normy, 0.0f, 1.0f);

	return SampleHeightmap(static_cast<int>(x * HeightmapSize),
	                       static_cast<int>(y * HeightmapSize));
}

unsigned char PlaneMeshGeneratorComponent::WrapSampleHeightmap(int x, int y) const
{
	if (HeightmapSize == 0)
		return 0;

	// Handle underflow
	if (x < 0)
		x = HeightmapSize - x;
	if (y < 0)
		y = HeightmapSize - y;

	// Handle overflow
	x %= HeightmapSize;
	y %= HeightmapSize;

	const int index = y * HeightmapSize + x;
	if (index >= Heightmap.size())
		throw std::out_of_range("Current values somehow still out of range.");

	return Heightmap[index];
}

unsigned char PlaneMeshGeneratorComponent::WrapSampleHeightmap(float normx, float normy) const
{
	return WrapSampleHeightmap(static_cast<int>(normx * HeightmapSize),
	                           static_cast<int>(normy * HeightmapSize));
}

DirectX::SimpleMath::Vector3 PlaneMeshGeneratorComponent::CalculateNormalAt(int x, int y) const
{
	if (!HeightmapSize)
		return DirectX::SimpleMath::Vector3::Up;

	const unsigned char sx = SampleHeightmap(x % HeightmapSize, y) - SampleHeightmap(x != 0 ? x - 1 : HeightmapSize - 1, y);
	const unsigned char sy = SampleHeightmap(x, y % HeightmapSize) - SampleHeightmap(x, y != 0 ? y - 1 : HeightmapSize - 1);

	DirectX::SimpleMath::Vector3 normal(-static_cast<float>(sx), 2.0f, static_cast<float>(sy));
	normal.Normalize();

	return normal;
}

DirectX::SimpleMath::Vector3 PlaneMeshGeneratorComponent::CalculateNormalAt(float normx, float normy) const
{
	const float x = std::clamp(normx, 0.0f, 1.0f);
	const float y = std::clamp(normy, 0.0f, 1.0f);

	return CalculateNormalAt(static_cast<int>(x * HeightmapSize),
	                         static_cast<int>(y * HeightmapSize));
}
