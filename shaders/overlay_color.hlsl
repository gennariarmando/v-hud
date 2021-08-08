sampler screen : register(s0);
sampler color : register(s1);

float3 HUEtoRGB(in float H)
{
    float R = abs(H * 6 - 3) - 1;
    float G = 2 - abs(H * 6 - 2);
    float B = 2 - abs(H * 6 - 4);
    return saturate(float3(R,G,B));
}

float3 HSLtoRGB(in float3 HSL)
{
    float3 RGB = HUEtoRGB(HSL.x);
    float C = (1 - abs(2 * HSL.z - 1)) * HSL.y;
    return (RGB - 0.5) * C + HSL.z;
}

float RGBCVtoHUE(in float3 RGB, in float C, in float V)
{
    float3 Delta = (V - RGB) / C;
    Delta.rgb -= Delta.brg;
    Delta.rgb += float3(2,4,6);
    Delta.brg = step(V, RGB) * Delta.brg;
    float H = max(Delta.r, max(Delta.g, Delta.b));
    return frac(H / 6);
}

float3 RGBtoHSL(in float3 RGB)
{
    float3 HSL = 0;
    float U, V;
    U = -min(RGB.r, min(RGB.g, RGB.b));
    V = max(RGB.r, max(RGB.g, RGB.b));
    HSL.z = (V - U) * 0.5;
    float C = V + U;
    if (C != 0)
    {
      HSL.x = RGBCVtoHUE(RGB, C, V);
      HSL.y = C / (1 - abs(2 * HSL.z - 1));
    }
    return HSL;
}

float4 main(float2 tex : TEXCOORD0) : COLOR0
{
    float4 scr_rgba = tex2D(screen, tex);
    float3 scr_hsl = RGBtoHSL(scr_rgba.rgb);
    float3 tex_hsl = RGBtoHSL(tex2D(color, tex).rgb);
    float3 fin_hsl = float3(tex_hsl.x, tex_hsl.y, scr_hsl.z);
    return float4(HSLtoRGB(fin_hsl), scr_rgba.a);   
}