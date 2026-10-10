cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 position = float4(input.position, 1.0f);

    position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);

    output.position = position;
    output.color = input.color;
    output.uv = input.uv;

    return output;
}