struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float alpha : COLOR1;
    float3 conic : COLOR2;
    float2 coordxy : COLOR3;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    //return float4(input.color, 0.5);
    float power = -0.5f * (input.conic.x * input.coordxy.x * input.coordxy.x + input.conic.z * input.coordxy.y * input.coordxy.y) - input.conic.y * input.coordxy.x * input.coordxy.y;
    
    if (power > 0.0f)
        discard;
    
    float opacity = min(0.99f, input.alpha * exp(power));
    
    if (opacity < 1.0f / 255.0f)
        discard;
    
    return float4(input.color.rgb, opacity);
}
