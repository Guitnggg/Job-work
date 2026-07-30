#pragma pack_matrix(row_major)

cbuffer ViewProjection : register(b0)
{
    matrix view;
    matrix projection;
    float elapsedTime;
    float3 constantsPadding;
};

struct Particle
{
    float3 initialPosition;
    float spawnTime;
    float3 velocity;
    float life;
    float startScale;
    float endScale;
    float active;
    float padding;
    float4 startColor;
    float4 endColor;
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
    float3 worldPos = p.initialPosition;
    float age = max(elapsedTime - p.spawnTime, 0.0f);

    if (p.active > 0.5f && p.life > 0.0f && age < p.life)
    {
        float normalizedAge = saturate(age / p.life);
        alpha = 1.0f - normalizedAge;
        scale = max(lerp(p.startScale, p.endScale, normalizedAge), 0.0f);
        worldPos = p.initialPosition + p.velocity * age;
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
    float t = (p.life > 0.0f) ? saturate(age / p.life) : 1.0f;
    float4 particleColor = lerp(p.startColor, p.endColor, t);
    output.color = float4(particleColor.rgb, particleColor.a * alpha);
    return output;
}
