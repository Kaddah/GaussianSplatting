struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 offset : OFFSET;
    float opacity : TEXCOORD18;
    float3 conic : TEXCOORD19;
    float3 hfovxy_focal : TEXCOORD20;
    float2 coordxy : TEXCOORD21;
};

cbuffer Uniforms
{
    int render_mod;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 color = input.color.rgb;
    float alpha = input.opacity;
    float3 conic = input.conic;
    float2 coordxy = input.coordxy; // Assuming offset as coordxy
    float4 FragColor = float4(color, alpha);

    if (render_mod == -2)
    {
        FragColor = float4(color, 1.0f);
        return FragColor;
    }
    
    float power = -0.5f * (conic.x * coordxy.x * coordxy.x + conic.z * coordxy.y * coordxy.y) - conic.y * coordxy.x * coordxy.y;
    
    if (power > 0.0f)
        discard;
    
    float computedOpacity = min(0.99f, alpha * exp(power));
    
    if (computedOpacity < 1.0f / 255.0f)
        discard;
    
    FragColor = float4(color, computedOpacity);
    
    if (render_mod == -3)
    {
        FragColor.a = FragColor.a > 0.22f ? 1.0f : 0.0f;
    }
    else if (render_mod == -4)
    {
        FragColor.a = FragColor.a > 0.22f ? 1.0f : 0.0f;
        FragColor.rgb = FragColor.rgb * exp(power);
    }

    return FragColor;
}
