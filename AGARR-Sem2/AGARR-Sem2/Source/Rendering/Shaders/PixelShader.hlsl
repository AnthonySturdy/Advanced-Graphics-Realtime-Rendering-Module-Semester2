Texture2D tx0 : register(t0);
Texture2D tx1 : register(t1);
Texture2D tx2 : register(t2);
Texture2D tx3 : register(t3);
Texture2D tx4 : register(t4);
Texture2D tx5 : register(t5);
Texture2D tx6 : register(t6);
Texture2D tx7 : register(t7);
Texture2D tx8 : register(t8);
Texture2D tx9 : register(t9);

SamplerState Sampler : register(s0);

#define MAX_TEXTURES 10

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

float EaseInOutExponential(float x)
{
    return x < 0.5 ? 16 * x * x * x * x * x : 1 - pow(-2 * x + 2, 5) / 2;
}

float4 main(PS_INPUT Input) : SV_TARGET
{
    float texScale = 2.0f;
    const float2 tc = float2(modf(Input.WorldPos.x, texScale), modf(Input.WorldPos.z, texScale));

    float4 texSamples[MAX_TEXTURES];
    texSamples[0] = tx0.Sample(Sampler, tc);
    texSamples[1] = tx1.Sample(Sampler, tc);
    texSamples[2] = tx2.Sample(Sampler, tc);
    texSamples[3] = tx3.Sample(Sampler, tc);
    texSamples[4] = tx4.Sample(Sampler, tc);
    texSamples[5] = tx5.Sample(Sampler, tc);
    texSamples[6] = tx6.Sample(Sampler, tc);
    texSamples[7] = tx7.Sample(Sampler, tc);
    texSamples[8] = tx8.Sample(Sampler, tc);
    texSamples[9] = tx9.Sample(Sampler, tc);

    const float normHeight = Input.WorldPos.y / HeightmapScale;
    float individualTexHeight = 1.0f / NumTextures;
    const int texIndex = floor(normHeight / individualTexHeight);
    const float individualTexNorm = fmod(normHeight, individualTexHeight) / individualTexHeight;

    const float diffuse = dot(Input.Normal, normalize(float3(2.0f, 8.0f, 4.0f)));
    const float ambient = 0.2f;

    if (NumTextures == 1)
        return texSamples[0] * max(diffuse, ambient);

    return lerp(texSamples[texIndex], 
				texSamples[min(texIndex + 1, NumTextures - 1)], 
				EaseInOutExponential(individualTexNorm))
			* max(diffuse, ambient);
}