#version 430 core

struct Light{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant; 
    float linear;
    float quadratic;
};

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragmentPosition; 

out vec4 color;

uniform Light light; 



float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{
    // amibient

    // disfuse

    // specular

    // attenuation

    color = vec4(0.5 * normal + 0.5, 1.0);
}
