    struct VS_OUTPUT
    {
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

float4 main(VS_OUTPUT input)
    : SV_TARGET
{
    static const float PI       = 3.14159265f;
    float3             cam_pos   = {0.5, 0.5, 0.5};
    float3             direction = input.pos.xyz - cam_pos;
    direction                    = normalize(direction);

    float  r      = sqrt(pow(direction.x, 2) + pow(direction.y, 2) + pow(direction.z, 2));
    float theta = acos(direction.z / r);
    float  phi    = atan2(direction.y, direction.x);

    // Spherical Harmonic Y_1^1
    float sh1_1 = -sqrt(3.0 / (8.0 * PI)) * sin(theta) * cos(phi);
    // Spherical Harmonic Y_2^0
    float sh2_0 = (1.0 / 4.0)  * sqrt(5.0 / PI) * (3 * pow(cos(theta),2)-1);
    // Spherical Harmonic Y_2^2
    float sh = (1.0 / 4.0) * sqrt(15 / PI) * pow(sin(theta),2)*sin(2*phi);
    float4 sh_color = {abs(sh), abs(sh1_1), abs(sh2_0), 1.0};
    //float4 sh_color = {0.0, abs(sh1_1), 0.0, 1.0};
    // return interpolated color
    // return input.color;
    return sh_color;
}
