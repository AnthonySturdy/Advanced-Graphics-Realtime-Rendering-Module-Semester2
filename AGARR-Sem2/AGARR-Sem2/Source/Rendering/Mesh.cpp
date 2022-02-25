#include "pch.h"
#include "Mesh.h"

#include "WaveFrontReader.h"

using namespace DirectX::SimpleMath;

Mesh::Mesh()
{
	Initialise(DefaultCubeObjPath);
}

void Mesh::Initialise(std::wstring modelPath)
{
	LoadDataFromOBJ(modelPath);

	CreateMeshBuffers();
}

void Mesh::Initialise(int width, int height)
{
	std::vector<Vertex> newVerts;
	std::vector<unsigned short> newIndices;

	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			// Generate vertices
			Vertex vert = {
				Vector3(x, 0, y),
				Vector3(0, 1, 0),
				Vector2(x, y)
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

	Vertices = newVerts;
	Indices = newIndices;

	CreateMeshBuffers();
}

void Mesh::CreateMeshBuffers()
{
	const auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * Vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = Vertices.data();

	DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, VertexBuffer.ReleaseAndGetAddressOf()));

	// Create index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned short) * Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	InitData.pSysMem = Indices.data();

	DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, IndexBuffer.ReleaseAndGetAddressOf()));
}

void Mesh::LoadDataFromOBJ(std::wstring path)
{
	WaveFrontReader<unsigned short> objReader{};
	objReader.Load(path.c_str(), true);
	Vertices = objReader.vertices;
	Indices = objReader.indices;
}
