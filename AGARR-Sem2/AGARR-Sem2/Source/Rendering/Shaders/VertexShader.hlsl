struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

float4 main(VS_INPUT Input) : SV_POSITION
{
	return Input.Pos;
}