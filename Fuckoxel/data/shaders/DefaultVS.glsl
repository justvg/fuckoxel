#version 330 core
layout (location = 0) in vec4 aP;
layout (location = 1) in vec4 aColor;

uniform mat4 Projection = mat4(1.0);
uniform mat4 View = mat4(1.0);
uniform mat4 Model = mat4(1.0);

out vs_data
{   
    vec3 FragWorldP;
    vec3 Color;
    float LightIntensity;
    float Occlusion;
} Output;

void main()
{
    Output.FragWorldP = vec3(Model * vec4(vec3(aP), 1.0));
    Output.Color = vec3(aColor);
    Output.LightIntensity = aColor.w;
    Output.Occlusion = aP.w;
    
    gl_Position = Projection * View * vec4(Output.FragWorldP, 1.0);
}