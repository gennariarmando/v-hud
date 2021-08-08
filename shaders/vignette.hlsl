sampler s0 : register(s0);
float4 p0 : register(c0);
float4 p1 : register(c1);

float4 vignette(float4 c, float2 tex)
{
	float2 tc = tex - float2(0.500, 0.500);	
	tc *= float2((p1.y / p1.x), 1.0);
	tc /= 1.0;
	float v = dot(tc, tc);
	c.rgb *= (1.0 + pow(v, 1.0) * -0.8); //pow - multiply
	return c;
}

float4 main(float2 tex : TEXCOORD0) : COLOR {
	float4 c0 = tex2D(s0, tex);

	c0 = vignette(c0, tex);
	return c0;
}