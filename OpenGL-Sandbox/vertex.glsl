#version 430
in vec2 position;
out vec2 coord;

void main()
{
    coord = position * 0.5 + 0.5;
    gl_Position = vec4(position.x, position.y, 0, 1.0);
}
//