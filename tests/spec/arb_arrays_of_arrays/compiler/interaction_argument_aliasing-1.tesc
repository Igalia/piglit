/* [config]
 * expect_result: pass
 * glsl_version: 430
 * [end config]
 */
#version 430 core

layout(vertices = 1) out;

out float tcs_result[];

bool gfunc(dmat4 x[2][2][2][2][2][2][2][2], dmat4 y[2][2][2][2][2][2][2][2])
{ 
        for (uint a = 0u; a < 2u; a++)
    {
        for (uint b = 0u; b < 2u; b++)
        {
            for (uint c = 0u; c < 2u; c++)
            {
                for (uint d = 0u; d < 2u; d++)
                {
                    for (uint e = 0u; e < 2u; e++)
                    {
                        for (uint f = 0u; f < 2u; f++)
                        {
                           for (uint g = 0u; g < 2u; g++)
                           {
                               for (uint h = 0u; h < 2u; h++)
                               {
                               x[a][b][c][d][e][f][g][h] = dmat4(123);
                               }
                            }
                        }
                    }
                }
            }
        }
    }

        for (uint a = 0u; a < 2u; a++)
    {
        for (uint b = 0u; b < 2u; b++)
        {
            for (uint c = 0u; c < 2u; c++)
            {
                for (uint d = 0u; d < 2u; d++)
                {
                    for (uint e = 0u; e < 2u; e++)
                    {
                        for (uint f = 0u; f < 2u; f++)
                        {
                           for (uint g = 0u; g < 2u; g++)
                           {
                               for (uint h = 0u; h < 2u; h++)
                               {
                                   if(y[a][b][c][d][e][f][g][h][0][0] != double(((a*128u)+(b*64u)+(c*32u)+(d*16u)+(e*8u)+(f*4u)+(g*2u)+h))) {return false;}
                               }
                           }
                        }
                    }
                }
            }
        }
    }
  return true;
}

void main()
{
    dmat4 z[2][2][2][2][2][2][2][2];

        for (uint a = 0u; a < 2u; a++)
    {
        for (uint b = 0u; b < 2u; b++)
        {
            for (uint c = 0u; c < 2u; c++)
            {
                for (uint d = 0u; d < 2u; d++)
                {
                    for (uint e = 0u; e < 2u; e++)
                    {
                        for (uint f = 0u; f < 2u; f++)
                        {
                           for (uint g = 0u; g < 2u; g++)
                           {
                               for (uint h = 0u; h < 2u; h++)
                               {
                                   z[a][b][c][d][e][f][g][h] = dmat4(((a*128u)+(b*64u)+(c*32u)+(d*16u)+(e*8u)+(f*4u)+(g*2u)+h));
                                   }
                           }
                        }
                    }
                }
            }
        }
    }


    float result = 0.0;
    if(gfunc(z, z) == true)
    {
        result = 1.0;

    }
    else 
    {
        result = 0.0;

    }


    tcs_result[gl_InvocationID] = result;

    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0; 
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;
}
