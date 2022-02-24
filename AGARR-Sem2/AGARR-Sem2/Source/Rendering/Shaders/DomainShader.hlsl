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

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
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

[domain("tri")]
PS_INPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	PS_INPUT Output;

    Output.Pos = float4((patch[0].Pos * domain.x +
						patch[1].Pos * domain.y +
						patch[2].Pos * domain.z).xyz,
						1.0f);
    Output.Pos = mul(Output.Pos, World);
    Output.Pos = mul(Output.Pos, View);
    Output.Pos = mul(Output.Pos, Projection);

    Output.Normal = float3(patch[0].Norm * domain.x +
						   patch[1].Norm * domain.y +
						   patch[2].Norm * domain.z);

    Output.TexCoord = float2(patch[0].TexCoord * domain.x +
							 patch[1].TexCoord * domain.y);

	return Output;
}
