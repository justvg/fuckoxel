#version 330 core
out vec4 FragColor;

in vs_data
{   
    vec3 FragWorldP;
    vec3 Color;
    float LightIntensity;
    float Occlusion;
} Input;

uniform vec2 Resolution;

vec3 ApplyFog(vec3 SourceColor, float Distance, vec3 ToFragDir, vec3 ToSunDir)
{
	const float e = 2.71828182845904523536028747135266249;
    const vec3 FogColor = vec3(0.2, 0.4, 0.8);
    const vec3 SunColor = vec3(1.0, 0.9, 0.7);

    float SunAmount = (1.0 - pow(e, -Distance*0.125))*max(dot(ToFragDir, ToSunDir), 0.0);
    vec3 FinalFogColor = mix(FogColor, SunColor, pow(SunAmount, 64.0));

    float FogAmount = 1.0 - pow(e, -pow(Distance*0.009, 2));
    vec3 Result = mix(SourceColor, FinalFogColor, FogAmount);
    return(Result);
}

void main()
{
    vec2 FragCoord = gl_FragCoord.xy;
    vec2 UV = vec2(FragCoord.x / Resolution.x, FragCoord.y / Resolution.y);
    float DistanceFromCenter = length(UV - vec2(0.5, 0.5));
    float X = 2.0*DistanceFromCenter - 0.55;
    X = clamp(1.219512*X, 0.0, 1.0);

    float X2 = X * X;
    float X3 = X * X2;
    float X4 = X2 * X2;

    float FinalX = pow(dot(vec4(X4, X3, X2, X), vec4(-0.1, -0.105, 1.12, 0.09)), 1);
    FinalX = min(FinalX, 0.94);

    vec3 FinalColor = (1.0 - FinalX)*Input.Occlusion*Input.LightIntensity*Input.Color;
    // vec3 FinalColor = (1.0 - FinalX)*Input.Occlusion*Input.Color;
    FinalColor = ApplyFog(FinalColor, length(Input.FragWorldP), normalize(Input.FragWorldP), -normalize(vec3(-0.5, -0.5, -0.5)));

    FragColor = vec4(FinalColor, 1.0);
}