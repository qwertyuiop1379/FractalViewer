#version 430
in vec2 coord;
out vec4 color;
uniform uint iter_max;
uniform uint mode;
uniform uvec2 screen;
uniform dvec2 center;
uniform double zoom;

uint check_point(dvec2 c)
{
    uint i = iter_max;
    dvec2 z;
    
    if (mode == 0)
    {
        z = vec2(0, 0);

        while (z.x * z.x + z.y * z.y < 4)
        {
            z = dvec2(z.x * z.x - z.y * z.y + c.x, z.x * z.y * 2 - c.y);

            if (--i == 0)
                return 0;
        }
    }
    else
    {
        z = vec2(0, 0);
        
        while (z.x * z.x + z.y * z.y < 4)
        {
            z = abs(dvec2(z.x * z.x - z.y * z.y + c.x, z.x * z.y * 2 - c.y));

            if (--i == 0)
                return 0;
        }
    }

    return iter_max - i + 1;
}

void main()
{
    dvec2 st = screen / zoom;
    uint pc = check_point(center + coord * st - st / 2);

    color = vec4(0.0, 0.0, 0.0, 1.0);

    if (pc > 0)
    {
        float gm = log(pc / 3);
        color.x = gm - floor(gm);
        gm = log(gm);
        color.y = gm - floor(gm);
        gm = log(pc / 2);
        color.z = gm - floor(gm);
    }
}
//