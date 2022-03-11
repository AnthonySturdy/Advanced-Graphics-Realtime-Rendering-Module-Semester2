#include "pch.h"
#include "Mesh.h"

#include "WaveFrontReader.h"

using namespace DirectX::SimpleMath;

Mesh::Mesh()
{
	Initialise(DefaultCubeObjPath);
}

void Mesh::Initialise(const std::wstring& modelPath)
{
	LoadDataFromOBJ(modelPath);

	CreateMeshBuffers();
}

void Mesh::Initialise(const std::vector<Vertex>& newVerts, const std::vector<UINT>& newIndices)
{
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
	bd.ByteWidth = sizeof(UINT) * Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	InitData.pSysMem = Indices.data();

	DX::ThrowIfFailed(device->CreateBuffer(&bd, &InitData, IndexBuffer.ReleaseAndGetAddressOf()));
}

void Mesh::LoadDataFromOBJ(const std::wstring& path)
{
	WaveFrontReader<UINT> objReader{};
	objReader.Load(path.c_str(), true);
	Vertices = objReader.vertices;
	Indices = objReader.indices;
}
