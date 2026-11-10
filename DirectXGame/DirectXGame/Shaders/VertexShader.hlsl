StructuredBuffer<matrix> bones : register(t0);

cbuffer MatrixBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix meshNode;
};

cbuffer BoneInfoBuffer : register(b1)
{
    uint boneCount;
    float3 padding;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    int4 boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

float4x4 CalculateBlendMatrix(int4 boneIndices, float4 boneWeights)
{
    float4x4 blend = (float4x4)0;
    float totalWeight = 0.0f;

    for (int i = 0; i < 4; ++i)
    {
        int index = boneIndices[i];
        float weight = boneWeights[i];

        if (index < 0 || (uint)index >= boneCount || weight <= 0.0f)
            continue;

        blend += bones[index] * weight;
        totalWeight += weight;
    }

    if (totalWeight <= 0.0f)
        return meshNode;

    return blend / totalWeight;
}

VSOutput main(VSInput input)
{
    VSOutput output;
    float4 position = float4(input.position, 1.0f);
    float4 normal = float4(input.normal, 0.0f);
    float4x4 blend = CalculateBlendMatrix(input.boneIndices, input.boneWeights);

    // TODO: 애니메이션 구현 완료 후 Normal 변환 개선
    // - 비균등 스케일 대응
    // - World Normal Matrix 역전치 적용
    // - Bone Normal Matrix 처리 방식 검토
    // - 조명 및 Normal 렌더링 검증
    normal = mul(normal, blend);
    normal = mul(normal, world);

    position = mul(position, blend);
    position = mul(position, world);
    position = mul(position, view);
    position = mul(position, projection);



    output.position = position;
    output.uv = input.uv;
    output.normal = normalize(normal.xyz);

    return output;
}