    struct VS_OUTPUT
    {
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
<<<<<<< Updated upstream
    // return interpolated color
    return input.color;
}
=======
   
    // return interpolated color
     //return input.color;
    //return sh_color;
}
>>>>>>> Stashed changes
