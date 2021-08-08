sampler screen : register(s0);
float stage : register(c0);

float4 main(float2 tex : TEXCOORD0) : COLOR0
{
    float4 color = tex2D(screen, tex) * (1.0 + stage);
    float d = dot(color.xyz, float3(0.3, 0.59, 0.11)).r;
    return float4(color.r + (d - color.r) * stage, color.g + (d - color.g) * stage, color.b + (d - color.b) * stage, color.a);  
}