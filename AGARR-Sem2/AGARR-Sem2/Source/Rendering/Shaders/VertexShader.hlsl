cbuffer CameraConstantBuffer : register(b0)
{
    matrix View;
    matrix Projection;
}

cbuffer PerObjectConstantBuffer : register(b1)
{
    matrix World;
    float TessellationAmount;
}
  
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct HS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

HS_INPUT main(VS_INPUT Input)
{
    HS_INPUT output = (HS_INPUT) 0;
    output.Pos = Input.Pos;
    output.Norm = Input.Norm;
    output.TexCoord = Input.Tex;

	return output;
}