struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // Multiply the color with its alpha component to achieve transparency
    float4 blendedColor = float4(input.color.rgb * input.color.a, 0.5);
    
    // Optionally, apply additional blending or effects here
    
    return blendedColor;
}
