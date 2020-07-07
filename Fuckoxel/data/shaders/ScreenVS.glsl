#version 330 core
layout (location = 0) in vec2 aP;

uniform mat4 Projection = mat4(1.0);
uniform mat4 Model = mat4(1.0);

out vec2 TexCoords;

void main()
{
    TexCoords = aP + vec2(0.5, 0.5);

    gl_Position = Projection * Model * vec4(aP, 1.0, 1.0);
}