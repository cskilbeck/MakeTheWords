/////

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer Projection : register(b0)
{
	matrix projection;
};

cbuffer ConstColor : register(b4)
{
	float4 constColor;
};

struct VS_INPUT
{
    float2 Pos : SV_Position;
    float2 Tex : TEXCOORD0;
	float4 Col : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_Position;
    float2 Tex : TEXCOORD0;
	float4 Col : COLOR;
};

PS_INPUT SpriteVertexShader(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;

	output.Pos = mul(float4(input.Pos.x, input.Pos.y, 0.5, 1.0), projection);

    output.Tex = input.Tex;
	output.Col = input.Col;
    return output;
}

float4 SpritePixelShader(PS_INPUT input) : SV_Target
{
	return (txDiffuse.Sample(samLinear, input.Tex)) * input.Col;
}
