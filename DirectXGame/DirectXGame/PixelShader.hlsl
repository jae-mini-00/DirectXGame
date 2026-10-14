Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MaterialBuffer : register(b0)
{
    float4 baseColor;
    int hasTexture;
    int padding[3];
};

cbuffer LightBuffer : register(b1)
{
    float3 direction;
    float intensity;
    float4 color;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(direction);
    float diffuse = max(dot(normal, -lightDir), 0.0f);
    float4 textureColor = baseColor * color * intensity * diffuse;
    if (hasTexture)
        textureColor *= texture0.Sample(sampler0, input.uv);

    return textureColor;
}