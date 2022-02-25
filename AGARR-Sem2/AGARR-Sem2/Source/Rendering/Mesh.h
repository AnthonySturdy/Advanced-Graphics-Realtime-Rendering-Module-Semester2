#pragma once

struct Vertex
{
	DirectX::SimpleMath::Vector3 Pos{ DirectX::SimpleMath::Vector3::Zero };
	DirectX::SimpleMath::Vector3 Normal{ DirectX::SimpleMath::Vector3::Up };
	DirectX::SimpleMath::Vector2 TexCoord{ DirectX::SimpleMath::Vector2::Zero };
};

class Mesh
{
public:
	Mesh();
	Mesh(const Mesh&) = default;
	Mesh(Mesh&&) = default;
	Mesh& operator=(const Mesh&) = default;
	Mesh& operator=(Mesh&&) = default;
	~Mesh() = default;

	void Initialise(std::wstring modelPath);
	void Initialise(int width, int height);

	[[nodiscard]] ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer.Get(); }
	[[nodiscard]] ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer.Get(); }
	[[nodiscard]] int GetNumIndices() const { return Indices.size(); }

private:
	void CreateMeshBuffers();

	void LoadDataFromOBJ(std::wstring path);

	std::vector<Vertex> Vertices{};
	std::vector<unsigned short> Indices{};
	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer{};
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer{};

	static inline const std::wstring DefaultCubeObjPath{ L"Resources/cube.obj" };
};
