cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 position = float4(input.position, 1.0f);
    float4 normal = mul(float4(input.normal, 0.0f), world);

    position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);

    output.position = position;
    output.uv = input.uv;
    output.normal = normalize(normal.xyz);

    return output;
}