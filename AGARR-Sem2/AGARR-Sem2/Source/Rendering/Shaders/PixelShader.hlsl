struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PS_INPUT Input) : SV_TARGET
{
    return float4((Input.Normal + float3(1, 1, 1)) / 2, 1.0f);
}