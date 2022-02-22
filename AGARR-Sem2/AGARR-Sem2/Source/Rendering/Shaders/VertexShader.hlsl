cbuffer CameraConstantBuffer : register(b0)
{
    matrix View;
    matrix Projection;
}

cbuffer PerObjectConstantBuffer : register(b1)
{
    matrix World;
}
  
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

PS_INPUT main(VS_INPUT Input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Pos = mul(Input.Pos, World);
    output.WorldPos = output.Pos;
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);

    output.TexCoord = Input.Tex;
    
    output.Normal = mul(Input.Norm, (float3x3) World);
    output.Normal = normalize(output.Normal);

	return output;
}