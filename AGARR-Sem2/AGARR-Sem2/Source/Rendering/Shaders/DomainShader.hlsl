SamplerState Sampler : register(s0);

Texture2D<float> HeightmapTex : register(t0);

cbuffer CameraConstantBuffer : register(b0)
{
    matrix View;
    matrix Projection;
    float3 EyePos;
}

cbuffer PerObjectConstantBuffer : register(b1)
{
    matrix World;
    float TessellationAmount;
}

cbuffer TerrainConstantBuffer : register(b2)
{
    bool ApplyHeightmap;
    float HeightmapScale;
    uint NumTextures;
};

struct PS_INPUT
{
    float4 ScreenPos : SV_POSITION;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float4 Pos : POSITION0;
    float3 Norm : NORMAL0;
    float2 TexCoord : TEXCOORD0;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3] : SV_TessFactor;
	float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

float3 CalculateNormal(float2 texCoord)
{
    float sx = HeightmapTex.SampleLevel(Sampler, texCoord + float2(0.005f, 0), 0) - HeightmapTex.SampleLevel(Sampler, texCoord - float2(0.005f, 0), 0);
    float sy = HeightmapTex.SampleLevel(Sampler, texCoord + float2(0, 0.005f), 0) - HeightmapTex.SampleLevel(Sampler, texCoord - float2(0, 0.005f), 0);

    float3 normal = float3(sx * (HeightmapScale / 10.0f), 1.0f, sy * (HeightmapScale / 10.0f));
    return normalize(normal);
}

[domain("tri")]
PS_INPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	PS_INPUT Output;

    Output.Normal = float3(patch[0].Norm * domain.x +
						   patch[1].Norm * domain.y +
						   patch[2].Norm * domain.z);

    Output.TexCoord = float2(patch[0].TexCoord * domain.x +
							 patch[1].TexCoord * domain.y +
							 patch[2].TexCoord * domain.z).xy;

    Output.ScreenPos = float4((patch[0].Pos * domain.x +
						patch[1].Pos * domain.y +
						patch[2].Pos * domain.z).xyz,
						1.0f);

    if(ApplyHeightmap)
    {
        float4 texCol = HeightmapTex.SampleLevel(Sampler, Output.TexCoord, 0);
        Output.ScreenPos.y = texCol.r * HeightmapScale;

        Output.Normal = CalculateNormal(Output.TexCoord);

    }

    Output.WorldPos = mul(Output.ScreenPos, World);
    Output.ScreenPos = mul(Output.ScreenPos, World);
    Output.ScreenPos = mul(Output.ScreenPos, View);
    Output.ScreenPos = mul(Output.ScreenPos, Projection);

	return Output;
}
