cbuffer PerObjectConstantBuffer : register(b1)
{
    matrix World;
    float TessellationAmount;
}

struct HS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float depth : TEXCOORD1;
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

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HS_INPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	/***********************************************
	MARKING SCHEME: Use of Tessellation shaders with Level of Detail
	DESCRIPTION:	Adjusts tesselation factor to adjust LOD based on distance of patch from camera.
	***********************************************/
    Output.EdgeTessFactor[0] =
		Output.EdgeTessFactor[1] =
		Output.EdgeTessFactor[2] =
		Output.InsideTessFactor = clamp(TessellationAmount - (ip[0].depth / 2), 1, TessellationAmount);

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<HS_INPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.Pos = ip[i].Pos;
	Output.Norm = ip[i].Norm;
    Output.TexCoord = ip[i].TexCoord;

	return Output;
}
