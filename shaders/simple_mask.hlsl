sampler2D input : register(s0);
sampler2D Mask : register(s1);

float4 main(float2 uv : TEXCOORD) : COLOR 
{ 
    float4 color;
    color = tex2D(input,uv);    
    
    float4 mask = tex2D(Mask, uv);
    float grayscale = (mask.r + mask.g + mask.b) /3;
    
    color.r = (color.r / color.a) * grayscale;
    color.g = (color.g / color.a) * grayscale;
    color.b = (color.b / color.a) * grayscale;
    color.a = grayscale;
    
    return color;
}