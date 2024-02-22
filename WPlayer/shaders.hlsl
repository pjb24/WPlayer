
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D texture_y : register(t1);
Texture2D texture_u : register(t2);
Texture2D texture_v : register(t3);

SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 yuv;
    float4 output;

    const float3 offset = { -0.0627451017, -0.501960814, -0.501960814 };
    const float3 Rcoeff = { 1.164, 0.000, 1.596 };
    const float3 Gcoeff = { 1.164, -0.391, -0.813 };
    const float3 Bcoeff = { 1.164, 2.018, 0.000 };

    yuv.x = texture_y.Sample(g_sampler, input.uv).r;
    yuv.y = texture_u.Sample(g_sampler, input.uv).r;
    yuv.z = texture_v.Sample(g_sampler, input.uv).r;

    yuv += offset;

    output.r = dot(yuv, Rcoeff);
    output.g = dot(yuv, Gcoeff);
    output.b = dot(yuv, Bcoeff);
    output.a = 1.0f;
    
    return output;
    
    //return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
