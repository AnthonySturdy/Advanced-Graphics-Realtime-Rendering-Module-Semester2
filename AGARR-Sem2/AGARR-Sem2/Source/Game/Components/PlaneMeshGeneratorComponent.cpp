#include "pch.h"
#include "PlaneMeshGeneratorComponent.h"

#include "MeshRendererComponent.h"
#include "Rendering/Mesh.h"

void PlaneMeshGeneratorComponent::RenderGUI()
{
	// Static Heightmap
	// Heightmap file selection
	static char* path = new char[512]{};
	ImGui::InputText("##", path, 512, ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	if (ImGui::Button("Choose Heightmap"))
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

	// Heightmap controls
	static bool useHeightmap = false;
	if (HeightmapSize != 0)
		ImGui::Checkbox("Use Heightmap", &useHeightmap);
	else
		useHeightmap = false;

	static float heightMapScale = 1.0f;
	if (useHeightmap)
	{
		ImGui::DragFloat("Heightmap Scale", &heightMapScale, 0.001f, 0.001f, 50.0f);
		// TODO: Heightmap texturing per height, etc
	}

	// Plane Generation
	static int planeSz[2]{ 1, 1 };
	ImGui::DragInt2("Plane Size", &planeSz[0], 1, 1, useHeightmap ? HeightmapSize : INT16_MAX);
	if (ImGui::Button("Generate Plane"))
		GeneratePlane(planeSz[0], planeSz[1], useHeightmap, heightMapScale);
}

void PlaneMeshGeneratorComponent::GeneratePlane(int width, int height, bool useHeightmap, float heightMapScale)
{
	const auto meshRenderer = Parent->GetComponent<MeshRendererComponent>();
	if (!meshRenderer)
		return;

	Mesh* mesh = meshRenderer->GetMesh();

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
				heightMapSample = SampleHeightmap(normX, normY);

			// Generate vertices
			Vertex vert = {
				DirectX::SimpleMath::Vector3(x, heightMapSample * heightMapScale, y),
				CalculateNormalAt(normX, normY),
				DirectX::SimpleMath::Vector2(x, y)
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

DirectX::SimpleMath::Vector3 PlaneMeshGeneratorComponent::CalculateNormalAt(int x, int y) const
{
	float sx = SampleHeightmap(x < HeightmapSize - 1 ? x + 1 : x, y) - SampleHeightmap(x != 0 ? x - 1 : x, y);
	if (x == 0 || x == HeightmapSize - 1)
		sx *= 2;

	float sy = SampleHeightmap(x, y < HeightmapSize - 1 ? y + 1 : y) - SampleHeightmap(x, y != 0 ? y - 1 : y);
	if (y == 0 || y == HeightmapSize - 1)
		sy *= 2;

	DirectX::SimpleMath::Vector3 normal(-sx, 2, sy);
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
