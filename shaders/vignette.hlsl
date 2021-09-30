sampler s0 : register(s0);
float4 p0 : register(c0);

float4 vignette(float4 c, float2 tex) {
	float2 uv = tex.xy / p0.xy;
	uv *= 1.0f - uv.yx;
	float v = uv.x * uv.y * 15.0f;
	v = pow(v, 0.25f);
	c.rgb *= v;
	return c;
}

float4 main(float2 tex : TEXCOORD0) : COLOR {
	float4 c0 = tex2D(s0, tex);

	c0 = vignette(c0, tex);
	return c0;
}