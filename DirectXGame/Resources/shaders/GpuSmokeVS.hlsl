#pragma pack_matrix(row_major)

cbuffer ViewProjection : register(b0)
{
    matrix view;
    matrix projection;
    float4 color;
};

struct Particle
{
    float3 position;
    float scale;
    float3 velocity;
    float life;
    float age;
    float startScale;
    float endScale;
    float active;
};

StructuredBuffer<Particle> gParticles : register(t0);

struct VSInput
{
    float2 position : POSITION;
    uint instanceId : SV_InstanceID;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    Particle p = gParticles[input.instanceId];
    float alpha = 0.0f;
    float scale = 0.0f;
    float3 worldPos = p.position;

    if (p.active > 0.5f && p.life > 0.0f)
    {
        alpha = saturate(1.0f - (p.age / p.life));
        scale = p.scale;
    }
    else
    {
        alpha = 0.0f;
        scale = 0.0f;
    }

    float4 viewPos = mul(float4(worldPos, 1.0f), view);
    float2 offset = input.position * scale;
    viewPos.xy += offset;

    output.svpos = mul(viewPos, projection);
    output.color = float4(color.rgb, color.a * alpha);
    return output;
}